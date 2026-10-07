/* Copyright (C) 2026 The CyanogenMod Project
 * SPDX-License-Identifier: Apache-2.0
 */
#define LOG_TAG "MioneRIL"
#include <dlfcn.h>
#include <limits.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <cutils/log.h>
#include <telephony/ril.h>

#ifndef MIONE_VENDOR_RIL
#define MIONE_VENDOR_RIL "/vendor/lib/libril-qc-qmi-1.so"
#endif

static struct RIL_Env framework_env;
static struct RIL_Env vendor_env;
static const RIL_RadioFunctions *vendor_functions;
static RIL_RadioFunctions functions;
static void *vendor_library;

struct signal_request {
    RIL_Token token;
    struct signal_request *next;
};
static struct signal_request *signal_requests;
static pthread_mutex_t signal_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_once_t signal_log_once = PTHREAD_ONCE_INIT;

static void log_signal_layout(void)
{
    ALOGI("Signal strength: CAF v6 %zu bytes -> v8 %zu bytes",
          sizeof(RIL_SignalStrength_v6), sizeof(RIL_SignalStrength_v8));
}

/* Oreo's radio bridge accepts v8/v10 signal strength, but this v6 blob has no
 * LTE timing advance. Preserve every measured field and mark that field unknown.
 * Copy before calling libril: its legacy LTE conversion can modify the payload.
 */
static int convert_signal_strength(const void *data, size_t length,
                                   RIL_SignalStrength_v8 *result)
{
    RIL_SignalStrength_v6 legacy;
    if (data == NULL || length != sizeof(legacy))
        return 0;
    memcpy(&legacy, data, sizeof(legacy));
    result->GW_SignalStrength = legacy.GW_SignalStrength;
    result->CDMA_SignalStrength = legacy.CDMA_SignalStrength;
    result->EVDO_SignalStrength = legacy.EVDO_SignalStrength;
    memcpy(&result->LTE_SignalStrength, &legacy.LTE_SignalStrength,
           sizeof(legacy.LTE_SignalStrength));
    result->LTE_SignalStrength.timingAdvance = INT_MAX;
    pthread_once(&signal_log_once, log_signal_layout);
    return 1;
}

static int take_signal_request(RIL_Token token)
{
    struct signal_request **link, *pending = NULL;
    pthread_mutex_lock(&signal_lock);
    for (link = &signal_requests; *link != NULL; link = &(*link)->next) {
        if ((*link)->token == token) {
            pending = *link;
            *link = pending->next;
            break;
        }
    }
    pthread_mutex_unlock(&signal_lock);
    if (pending == NULL)
        return 0;
    free(pending);
    return 1;
}

static void on_complete(RIL_Token token, RIL_Errno error,
                        void *data, size_t length)
{
    RIL_SignalStrength_v8 signal;
    int is_signal = take_signal_request(token);
    if (is_signal && error == RIL_E_SUCCESS) {
        if (convert_signal_strength(data, length, &signal)) {
            framework_env.OnRequestComplete(token, error, &signal, sizeof(signal));
            return;
        }
        if (data == NULL || (length != sizeof(RIL_SignalStrength_v8) &&
                            length != sizeof(RIL_SignalStrength_v10)))
            ALOGE("Unexpected signal strength response: %zu bytes", length);
    }
    framework_env.OnRequestComplete(token, error, data, length);
}

/* The shipped CAF v6 library has no cell-info or initial-attach-APN request.
 * Its IMS and subscription requests predate their insertion into AOSP's table.
 * Reject unsupported requests before the blob can interpret their payload as
 * an unrelated command. Keep the blob's version; adapt signal strength below.
 */
static int vendor_request(int request)
{
    if (request > 0 && request <= 108)
        return request == 106 ? -1 : request;
    switch (request) {
    case RIL_REQUEST_IMS_REGISTRATION_STATE: return 109;
    case RIL_REQUEST_IMS_SEND_SMS: return 110;
#ifdef RIL_REQUEST_GET_DATA_CALL_PROFILE
    case RIL_REQUEST_GET_DATA_CALL_PROFILE: return 111;
#endif
    case RIL_REQUEST_SET_UICC_SUBSCRIPTION: return 112;
#ifdef RIL_REQUEST_SET_DATA_SUBSCRIPTION
    case RIL_REQUEST_SET_DATA_SUBSCRIPTION: return 113;
#endif
    /* Lollipop's ALLOW_DATA uses a different payload from CAF v6's
     * SET_DATA_SUBSCRIPTION. It must not be translated by number alone.
     */
    default: return -1;
    }
}

static void on_request(int request, void *data, size_t length, RIL_Token token)
{
    int translated = vendor_request(request);
    if (translated < 0) {
        framework_env.OnRequestComplete(token, RIL_E_REQUEST_NOT_SUPPORTED,
                                       NULL, 0);
        return;
    }
    if (request == RIL_REQUEST_SIGNAL_STRENGTH) {
        struct signal_request *pending = malloc(sizeof(*pending));
        if (pending == NULL) {
            framework_env.OnRequestComplete(token, RIL_E_GENERIC_FAILURE, NULL, 0);
            return;
        }
        pending->token = token;
        /* Completion can run synchronously or on another thread. Keep the
         * original token for the vendor's cancellation/acknowledgement paths.
         * Cancelled requests remain tracked until their completion arrives.
         */
        pthread_mutex_lock(&signal_lock);
        pending->next = signal_requests;
        signal_requests = pending;
        pthread_mutex_unlock(&signal_lock);
    }
    vendor_functions->onRequest(translated, data, length, token);
}

static int supports(int request)
{
    int translated = vendor_request(request);
    return translated >= 0 && vendor_functions->supports != NULL
        ? vendor_functions->supports(translated) : 0;
}

static void on_unsolicited(int event, const void *data, size_t length)
{
    RIL_SignalStrength_v8 signal;
    if (event == RIL_UNSOL_SIGNAL_STRENGTH &&
        convert_signal_strength(data, length, &signal)) {
        framework_env.OnUnsolicitedResponse(event, &signal, sizeof(signal));
        return;
    }
    switch (event) {
    case 1036: event = RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED; break;
    case 1038:
        /* CAF's void data-registration notification. The AOSP network-state
         * notification makes ServiceStateTracker poll both registration states.
         * Do not fabricate a data-call list or an SS result.
         */
        if (length != 0) {
            ALOGE("Invalid CAF data-network notification: %zu bytes", length);
            return;
        }
        event = RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED;
        ALOGI("CAF data-network state changed -> registration refresh");
        break;
    case 1039: event = RIL_UNSOL_ON_SS; break;
    case 1040: event = RIL_UNSOL_STK_CC_ALPHA_NOTIFY; break;
    case 1041: event = RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED; break;
    case 1037: /* CAF tethered-mode indication: no AOSP counterpart. */
    case 1042: /* CAF QoS indication. */
    case 1043: /* CAF modify-call indication. */
        ALOGI("Unsupported CAF extension %d (%zu bytes)", event, length);
        return;
    default: break;
    }
    framework_env.OnUnsolicitedResponse(event, data, length);
}

/* Check the actual blob contract, rather than assuming every v6 RIL uses CAF's
 * numbering. A different blob must be adapted explicitly.
 */
static int check_contract(void *library)
{
    const char *(*name)(int) = dlsym(library, "qcril_log_lookup_event_name");
    static const struct { int id; const char *name; } expected[] = {
        {1036, "RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED"},
        {1038, "RIL_UNSOL_RESPONSE_DATA_NETWORK_STATE_CHANGED"},
        {1039, "RIL_UNSOL_ON_SS"},
        {109, "RIL_REQUEST_IMS_REGISTRATION_STATE"},
        {110, "RIL_REQUEST_IMS_SEND_SMS"},
        {111, "RIL_REQUEST_GET_DATA_CALL_PROFILE"},
        {112, "RIL_REQUEST_SET_UICC_SUBSCRIPTION"},
        {113, "RIL_REQUEST_SET_DATA_SUBSCRIPTION"},
    };
    unsigned int i;
    if (name == NULL)
        return 0;
    for (i = 0; i < sizeof(expected) / sizeof(expected[0]); ++i) {
        const char *actual = name(expected[i].id);
        if (actual == NULL || strcmp(actual, expected[i].name) != 0)
            return 0;
    }
    return 1;
}

const RIL_RadioFunctions *RIL_Init(const struct RIL_Env *env,
                                 int argc, char **argv)
{
    const RIL_RadioFunctions *(*init)(const struct RIL_Env *, int, char **);
    if (vendor_library == NULL)
        vendor_library = dlopen(MIONE_VENDOR_RIL, RTLD_NOW | RTLD_LOCAL);
    if (vendor_library == NULL || !check_contract(vendor_library)) {
        ALOGE("Cannot load verified MiOne CAF RIL contract");
        return NULL;
    }
    init = dlsym(vendor_library, "RIL_Init");
    if (init == NULL)
        return NULL;
    framework_env = *env;
    vendor_env = *env;
    vendor_env.OnRequestComplete = on_complete;
    vendor_env.OnUnsolicitedResponse = on_unsolicited;
    vendor_functions = init(&vendor_env, argc, argv);
    if (vendor_functions == NULL || vendor_functions->version != 6) {
        ALOGE("MiOne adapter requires the verified CAF v6 RIL");
        return NULL;
    }
    functions = *vendor_functions;
    functions.onRequest = on_request;
    functions.supports = supports;
    ALOGI("Verified CAF v6 request/unsolicited numbering adapter initialized");
    return &functions;
}

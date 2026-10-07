/* SPDX-License-Identifier: Apache-2.0 */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <limits.h>
#include <pthread.h>
#include <string.h>
#include <telephony/ril.h>
#ifndef MIONE_TEST_DIR
#define MIONE_TEST_DIR "/data/local/tmp/mione-userspace"
#endif
static int failures, checks, last_event, unsol_count, last_error, complete_count;
static const void *last_data;
static size_t last_length;
static RIL_Token last_token;
static unsigned char last_payload[sizeof(RIL_SignalStrength_v10)];
static int mutate_signal;
#define CHECK(expr) do { ++checks; if (!(expr)) { ++failures; printf("FAIL line %d: %s\n", __LINE__, #expr); } } while (0)
static void complete(RIL_Token token, RIL_Errno error, void *data, size_t length)
{
    last_token = token; last_error = error; last_data = data; last_length = length;
    ++complete_count;
    if (data != NULL && length <= sizeof(last_payload))
        memcpy(last_payload, data, length);
    if (mutate_signal && data != NULL && length == sizeof(RIL_SignalStrength_v8))
        ((RIL_SignalStrength_v8 *)data)->LTE_SignalStrength.signalStrength = 99;
}
static void unsol(int id, const void *data, size_t length)
{
    last_event = id; last_data = data; last_length = length; ++unsol_count;
    if (data != NULL && length <= sizeof(last_payload))
        memcpy(last_payload, data, length);
    if (mutate_signal && data != NULL && length == sizeof(RIL_SignalStrength_v8))
        ((RIL_SignalStrength_v8 *)data)->LTE_SignalStrength.signalStrength = 99;
}
static void timer(RIL_TimedCallback cb, void *p, const struct timeval *tv)
{ (void)cb; (void)p; (void)tv; }

static void expect_signal(const RIL_SignalStrength_v6 *source)
{
    RIL_SignalStrength_v8 result;
    CHECK(last_length == sizeof(result));
    if (last_length != sizeof(result))
        return;
    memcpy(&result, last_payload, sizeof(result));
    CHECK(memcmp(&result, source, sizeof(*source)) == 0);
    CHECK(result.LTE_SignalStrength.timingAdvance == INT_MAX);
}

struct completion {
    void (*complete)(RIL_Token, RIL_Errno, void *, size_t);
    RIL_Token token;
    RIL_SignalStrength_v6 signal;
};
static void *worker_complete(void *data)
{
    struct completion *pending = data;
    pending->complete(pending->token, RIL_E_SUCCESS,
                      &pending->signal, sizeof(pending->signal));
    return NULL;
}
int main(void)
{
    void *adapter = dlopen(MIONE_TEST_DIR "/libmione_ril_test.so", RTLD_NOW);
    const RIL_RadioFunctions *(*init)(const struct RIL_Env *, int, char **);
    const RIL_RadioFunctions *f;
    void *fake;
    void (*emit)(int, const void *, size_t);
    int (*request)(void *, size_t, RIL_Token);
    void (*defer)(int);
    void (*finish)(RIL_Token, RIL_Errno, void *, size_t);
    int (*cancelled)(RIL_Token);
    struct RIL_Env env = {
        .OnRequestComplete = complete,
        .OnUnsolicitedResponse = unsol,
        .RequestTimedCallback = timer,
    };
    int payload = 42, token = 9, i;
    const int source[] = {1000,1002,1033,1035,1036,1038,1039,1040,1041};
    const int dest[] = {1000,1002,1033,1035,
        RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED,
        RIL_UNSOL_RESPONSE_VOICE_NETWORK_STATE_CHANGED,
        RIL_UNSOL_ON_SS, RIL_UNSOL_STK_CC_ALPHA_NOTIFY,
        RIL_UNSOL_UICC_SUBSCRIPTION_STATUS_CHANGED};
    const int req[] = {1,20,27,105,108,
        RIL_REQUEST_IMS_REGISTRATION_STATE, RIL_REQUEST_IMS_SEND_SMS,
#ifdef RIL_REQUEST_GET_DATA_CALL_PROFILE
        RIL_REQUEST_GET_DATA_CALL_PROFILE,
#endif
        RIL_REQUEST_SET_UICC_SUBSCRIPTION,
#ifdef RIL_REQUEST_SET_DATA_SUBSCRIPTION
        RIL_REQUEST_SET_DATA_SUBSCRIPTION,
#endif
    };
    const int target[] = {1,20,27,105,108,109,110,
#ifdef RIL_REQUEST_GET_DATA_CALL_PROFILE
        111,
#endif
        112,
#ifdef RIL_REQUEST_SET_DATA_SUBSCRIPTION
        113,
#endif
    };
    const int reject[] = {0,106,
        RIL_REQUEST_GET_CELL_INFO_LIST, RIL_REQUEST_SET_UNSOL_CELL_INFO_LIST_RATE,
        RIL_REQUEST_SET_INITIAL_ATTACH_APN, RIL_REQUEST_SIM_TRANSMIT_APDU_BASIC,
        RIL_REQUEST_SIM_OPEN_CHANNEL, RIL_REQUEST_SIM_CLOSE_CHANNEL,
        RIL_REQUEST_SIM_TRANSMIT_APDU_CHANNEL, RIL_REQUEST_NV_READ_ITEM,
        RIL_REQUEST_NV_WRITE_ITEM, RIL_REQUEST_NV_WRITE_CDMA_PRL,
        RIL_REQUEST_NV_RESET_CONFIG,
#ifdef RIL_REQUEST_ALLOW_DATA
        RIL_REQUEST_ALLOW_DATA, RIL_REQUEST_GET_HARDWARE_CONFIG,
        RIL_REQUEST_SIM_AUTHENTICATION, RIL_REQUEST_SET_DATA_PROFILE,
        RIL_REQUEST_SHUTDOWN,
#endif
#ifdef RIL_REQUEST_GET_RADIO_CAPABILITY
        RIL_REQUEST_GET_RADIO_CAPABILITY, RIL_REQUEST_SET_RADIO_CAPABILITY,
#endif
    };
    if (!adapter) { puts(dlerror()); return 1; }
    init = dlsym(adapter,"RIL_Init");
    if (!init) return 1;
    f = init(&env, 0, NULL);
    CHECK(f != NULL);
    if (!f) return 1;
    CHECK(f->version == 6); CHECK(last_event == 1002); CHECK(unsol_count == 1);
    fake = dlopen(MIONE_TEST_DIR "/libmione_fake_ril.so", RTLD_NOW);
    emit = dlsym(fake,"mione_test_emit"); request = dlsym(fake,"mione_test_request");
    defer = dlsym(fake,"mione_test_defer");
    finish = dlsym(fake,"mione_test_complete");
    cancelled = dlsym(fake,"mione_test_cancelled");
    if (!emit || !request || !defer || !finish || !cancelled) return 1;
    for (i=0; i<(int)(sizeof(source)/sizeof(source[0])); ++i) {
        int empty = source[i] == 1038;
        emit(source[i], empty ? NULL : &payload, empty ? 0 : sizeof(payload));
        CHECK(last_event == dest[i]);
        CHECK(last_data == (empty ? NULL : &payload));
        CHECK(last_length == (empty ? 0 : sizeof(payload)));
    }
    i=unsol_count; emit(1037,&payload,sizeof(payload)); emit(1042,&payload,sizeof(payload));
    emit(1043,&payload,sizeof(payload)); emit(1038,&payload,sizeof(payload));
    CHECK(unsol_count == i);
    for (i=0; i<(int)(sizeof(req)/sizeof(req[0])); ++i) {
        CHECK(f->supports(req[i]) == target[i]+100);
        f->onRequest(req[i], &payload, sizeof(payload), &token);
        CHECK(request(&payload,sizeof(payload),&token) == target[i]);
        CHECK(last_token == &token && last_data == &payload && last_length == sizeof(payload));
        CHECK(last_error == RIL_E_SUCCESS);
    }
    for (i=0; i<(int)(sizeof(reject)/sizeof(reject[0])); ++i) {
        int previous=complete_count;
        CHECK(f->supports(reject[i]) == 0);
        f->onRequest(reject[i],&payload,sizeof(payload),&token);
        CHECK(complete_count == previous+1 && last_token == &token);
        CHECK(last_error == RIL_E_REQUEST_NOT_SUPPORTED && last_data == NULL && last_length == 0);
    }
    {
        RIL_SignalStrength_v6 signal = {{17,3}, {75,125}, {82,100,7},
                                      {-1,-1,-1,INT_MAX,-1}};
        RIL_SignalStrength_v6 original = signal, other = signal;
        RIL_SignalStrength_v8 v8 = {{22,0}, {90,80}, {83,99,6},
                                   {19,88,7,120,8,12}};
        RIL_SignalStrength_v10 v10 = {{22,0}, {90,80}, {83,99,6},
                                     {19,88,7,120,8,12}, {67}};
        int tokens[3], previous, iteration;
        pthread_t worker;
        struct completion pending = {finish, &tokens[0], signal};
        CHECK(sizeof(signal) == 48 && sizeof(v8) == 52 && sizeof(v10) == 56);
        CHECK(f->supports(RIL_REQUEST_SIGNAL_STRENGTH) == RIL_REQUEST_SIGNAL_STRENGTH+100);

        /* Synchronous completion must already have its token registered. */
        mutate_signal = 1;
        previous = complete_count;
        f->onRequest(RIL_REQUEST_SIGNAL_STRENGTH, &signal, sizeof(signal), &token);
        CHECK(complete_count == previous+1 && last_token == &token);
        CHECK(last_error == RIL_E_SUCCESS);
        CHECK(request(&signal, sizeof(signal), &token) == RIL_REQUEST_SIGNAL_STRENGTH);
        expect_signal(&original);
        CHECK(memcmp(&signal, &original, sizeof(signal)) == 0);
        previous = unsol_count;
        emit(RIL_UNSOL_SIGNAL_STRENGTH, &signal, sizeof(signal));
        CHECK(unsol_count == previous+1 && last_event == RIL_UNSOL_SIGNAL_STRENGTH);
        expect_signal(&original);
        CHECK(memcmp(&signal, &original, sizeof(signal)) == 0);
        mutate_signal = 0;

        /* Current layouts and malformed payloads keep the original contract. */
        f->onRequest(RIL_REQUEST_SIGNAL_STRENGTH, &v8, sizeof(v8), &token);
        CHECK(last_data == &v8 && last_length == sizeof(v8));
        CHECK(memcmp(last_payload, &v8, sizeof(v8)) == 0);
        f->onRequest(RIL_REQUEST_SIGNAL_STRENGTH, &v10, sizeof(v10), &token);
        CHECK(last_data == &v10 && last_length == sizeof(v10));
        CHECK(memcmp(last_payload, &v10, sizeof(v10)) == 0);
        emit(RIL_UNSOL_SIGNAL_STRENGTH, &v8, sizeof(v8));
        CHECK(last_data == &v8 && last_length == sizeof(v8));
        emit(RIL_UNSOL_SIGNAL_STRENGTH, &v10, sizeof(v10));
        CHECK(last_data == &v10 && last_length == sizeof(v10));
        f->onRequest(RIL_REQUEST_SIGNAL_STRENGTH, &signal, sizeof(signal)-1, &token);
        CHECK(last_data == &signal && last_length == sizeof(signal)-1);
        f->onRequest(RIL_REQUEST_SIGNAL_STRENGTH, NULL, sizeof(signal), &token);
        CHECK(last_data == NULL && last_length == sizeof(signal));
        f->onRequest(RIL_REQUEST_SIGNAL_STRENGTH, NULL, 0, &token);
        CHECK(last_data == NULL && last_length == 0);
        emit(RIL_UNSOL_SIGNAL_STRENGTH, &signal, sizeof(signal)-1);
        CHECK(last_data == &signal && last_length == sizeof(signal)-1);
        emit(RIL_UNSOL_SIGNAL_STRENGTH, NULL, sizeof(signal));
        CHECK(last_data == NULL && last_length == sizeof(signal));
        f->onRequest(20, &signal, sizeof(signal), &token);
        CHECK(last_data == &signal && last_length == sizeof(signal));
        emit(1002, &signal, sizeof(signal));
        CHECK(last_data == &signal && last_length == sizeof(signal));

        /* Interleaved, out-of-order replies retain each original opaque token. */
        defer(1);
        previous = complete_count;
        f->onRequest(RIL_REQUEST_SIGNAL_STRENGTH, NULL, 0, &tokens[0]);
        f->onRequest(20, NULL, 0, &tokens[1]);
        f->onRequest(RIL_REQUEST_SIGNAL_STRENGTH, NULL, 0, &tokens[2]);
        CHECK(complete_count == previous);
        finish(&tokens[1], RIL_E_SUCCESS, &signal, sizeof(signal));
        CHECK(last_token == &tokens[1] && last_data == &signal && last_length == sizeof(signal));
        other.GW_SignalStrength.signalStrength = 7;
        finish(&tokens[2], RIL_E_SUCCESS, &other, sizeof(other));
        CHECK(last_token == &tokens[2]); expect_signal(&other);
        i = pthread_create(&worker, NULL, worker_complete, &pending);
        CHECK(i == 0);
        if (i == 0)
            CHECK(pthread_join(worker, NULL) == 0);
        CHECK(last_token == &tokens[0]); expect_signal(&signal);
        CHECK(complete_count == previous+3);

        /* Errors and cancellation must release tracking without inventing success. */
        f->onRequest(RIL_REQUEST_SIGNAL_STRENGTH, NULL, 0, &token);
        finish(&token, RIL_E_RADIO_NOT_AVAILABLE, &signal, sizeof(signal));
        CHECK(last_error == RIL_E_RADIO_NOT_AVAILABLE && last_data == &signal && last_length == sizeof(signal));
        f->onRequest(RIL_REQUEST_SIGNAL_STRENGTH, NULL, 0, &token);
        f->onCancel(&token); CHECK(cancelled(&token));
        finish(&token, RIL_E_CANCELLED, &signal, sizeof(signal));
        CHECK(last_error == RIL_E_CANCELLED && last_data == &signal && last_length == sizeof(signal));
        f->onRequest(20, NULL, 0, &token);
        finish(&token, RIL_E_SUCCESS, &signal, sizeof(signal));
        CHECK(last_data == &signal && last_length == sizeof(signal));

        /* Reuse a completed token repeatedly to expose stale tracking. */
        defer(0);
        previous = complete_count;
        for (iteration=0; iteration<64; ++iteration) {
            f->onRequest(RIL_REQUEST_SIGNAL_STRENGTH, &signal, sizeof(signal), &token);
            expect_signal(&signal);
            f->onRequest(20, &signal, sizeof(signal), &token);
            CHECK(last_data == &signal && last_length == sizeof(signal));
        }
        CHECK(complete_count == previous+128);
    }
    printf("RIL adapter: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}

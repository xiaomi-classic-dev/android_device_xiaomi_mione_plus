/* SPDX-License-Identifier: Apache-2.0 */
#include <telephony/ril.h>
#include <stddef.h>
static const struct RIL_Env *callbacks;
static int last_request;
static void *last_data;
static size_t last_length;
static RIL_Token last_token;
static void request(int id, void *data, size_t length, RIL_Token token)
{
    last_request = id; last_data = data; last_length = length; last_token = token;
    callbacks->OnRequestComplete(token, RIL_E_SUCCESS, data, length);
}
static int supports(int id) { last_request = id; return id + 100; }
static RIL_RadioFunctions functions = {6, request, NULL, supports, NULL, NULL};
const RIL_RadioFunctions *RIL_Init(const struct RIL_Env *env, int argc, char **argv)
{
    (void)argc; (void)argv; callbacks = env;
    /* Exercise callback lifetime during initialization, too. */
    callbacks->OnUnsolicitedResponse(1038, NULL, 0);
    return &functions;
}
const char *qcril_log_lookup_event_name(int id)
{
    switch (id) {
    case 1036: return "RIL_UNSOL_RESPONSE_IMS_NETWORK_STATE_CHANGED";
    case 1038: return "RIL_UNSOL_RESPONSE_DATA_NETWORK_STATE_CHANGED";
    case 1039: return "RIL_UNSOL_ON_SS";
    case 109: return "RIL_REQUEST_IMS_REGISTRATION_STATE";
    case 110: return "RIL_REQUEST_IMS_SEND_SMS";
    case 111: return "RIL_REQUEST_GET_DATA_CALL_PROFILE";
    case 112: return "RIL_REQUEST_SET_UICC_SUBSCRIPTION";
    case 113: return "RIL_REQUEST_SET_DATA_SUBSCRIPTION";
    default: return "unknown";
    }
}
void mione_test_emit(int id, const void *data, size_t length)
{ callbacks->OnUnsolicitedResponse(id, data, length); }
int mione_test_request(void *data, size_t length, RIL_Token token)
{ return last_data == data && last_length == length && last_token == token ? last_request : -1; }

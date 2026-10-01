/* SPDX-License-Identifier: Apache-2.0 */
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <telephony/ril.h>
static int failures, checks, last_event, unsol_count, last_error, complete_count;
static const void *last_data;
static size_t last_length;
static RIL_Token last_token;
#define CHECK(expr) do { ++checks; if (!(expr)) { ++failures; printf("FAIL line %d: %s\n", __LINE__, #expr); } } while (0)
static void complete(RIL_Token token, RIL_Errno error, void *data, size_t length)
{ last_token = token; last_error = error; last_data = data; last_length = length; ++complete_count; }
static void unsol(int id, const void *data, size_t length)
{ last_event = id; last_data = data; last_length = length; ++unsol_count; }
static void timer(RIL_TimedCallback cb, void *p, const struct timeval *tv)
{ (void)cb; (void)p; (void)tv; }
int main(void)
{
    void *adapter = dlopen("/data/local/tmp/mione-userspace/libmione_ril_test.so", RTLD_NOW);
    const RIL_RadioFunctions *(*init)(const struct RIL_Env *, int, char **);
    const RIL_RadioFunctions *f;
    void *fake;
    void (*emit)(int, const void *, size_t);
    int (*request)(void *, size_t, RIL_Token);
    struct RIL_Env env = {complete, unsol, timer};
    int payload = 42, token = 9, i;
    const int source[] = {1000,1002,1033,1035,1036,1038,1039,1040,1041};
    const int dest[] = {1000,1002,1033,1035,1037,1002,1038,1039,1040};
    const int req[] = {1,20,27,105,108,112,113,114,115,116};
    const int target[] = {1,20,27,105,108,109,110,111,112,113};
    const int reject[] = {0,106,109,110,111,117,118,119,120,121,122};
    if (!adapter) { puts(dlerror()); return 1; }
    init = dlsym(adapter,"RIL_Init");
    if (!init) return 1;
    f = init(&env, 0, NULL);
    CHECK(f != NULL);
    if (!f) return 1;
    CHECK(f->version == 6); CHECK(last_event == 1002); CHECK(unsol_count == 1);
    fake = dlopen("/data/local/tmp/mione-userspace/libmione_fake_ril.so", RTLD_NOW);
    emit = dlsym(fake,"mione_test_emit"); request = dlsym(fake,"mione_test_request");
    if (!emit || !request) return 1;
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
    printf("RIL adapter: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}

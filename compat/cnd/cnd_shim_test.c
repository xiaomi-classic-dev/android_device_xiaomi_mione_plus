/* SPDX-License-Identifier: Apache-2.0 */
#include "cnd_shim.h"
#include <cutils/jstring.h>
#include <assert.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int checks;
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "%s:%d: %s\n", \
    __FILE__, __LINE__, #x); return 1; } ++checks; } while (0)

static int convert(const char *source_name, const char *target_name,
                   const char *input, size_t input_size,
                   const char *expected, size_t expected_size, int chunked)
{
    UErrorCode status = U_ZERO_ERROR;
    UConverter *source_cnv = ucnv_open_46(source_name, &status);
    UConverter *target_cnv = ucnv_open_46(target_name, &status);
    UChar pivot[16], *pivot_source = pivot, *pivot_target = pivot;
    char output[64], *target = output;
    const char *source = input;
    int reset = 1;
    CHECK(U_SUCCESS(status) && source_cnv && target_cnv);
    do {
        char *limit = chunked ? target + 2 : output + sizeof(output);
        CHECK(limit <= output + sizeof(output));
        status = U_ZERO_ERROR;
        ucnv_convertEx_46(target_cnv, source_cnv, &target, limit,
                         &source, input + input_size, pivot, &pivot_source,
                         &pivot_target, pivot + 16, reset, 1, &status);
        reset = 0;
    } while (status == U_BUFFER_OVERFLOW_ERROR);
    CHECK(U_SUCCESS(status));
    CHECK(source == input + input_size);
    CHECK((size_t)(target - output) == expected_size);
    CHECK(memcmp(output, expected, expected_size) == 0);
    ucnv_close_46(source_cnv);
    ucnv_close_46(target_cnv);
    return 0;
}

static int callbacks(void)
{
    UErrorCode status = U_ZERO_ERROR;
    UConverter *utf8 = ucnv_open_46("UTF-8", &status);
    UConverter *ascii = ucnv_open_46("US-ASCII", &status);
    UConverterToUCallback old_to, restored_to;
    UConverterFromUCallback old_from, restored_from;
    const void *old_to_context, *old_from_context, *restored_context;
    static const int context = 42;
    UChar pivot[16], *ps = pivot, *pt = pivot;
    char output[32], *target = output;
    const char invalid[] = {(char)0xff}, chinese[] = "\xe4\xb8\xad";
    const char *source = invalid;
    CHECK(U_SUCCESS(status) && utf8 && ascii);
    ucnv_setToUCallBack_46(utf8, UCNV_TO_U_CALLBACK_STOP_46, &context,
                         &old_to, &old_to_context, &status);
    ucnv_setFromUCallBack_46(ascii, UCNV_FROM_U_CALLBACK_STOP_46, &context,
                           &old_from, &old_from_context, &status);
    CHECK(U_SUCCESS(status));
    ucnv_convertEx_46(ascii, utf8, &target, output + sizeof(output), &source,
                     invalid + sizeof(invalid), pivot, &ps, &pt, pivot + 16,
                     1, 1, &status);
    CHECK(U_FAILURE(status) && target == output);
    status = U_ZERO_ERROR;
    source = chinese; target = output; ps = pt = pivot;
    ucnv_convertEx_46(ascii, utf8, &target, output + sizeof(output), &source,
                     chinese + 3, pivot, &ps, &pt, pivot + 16, 1, 1, &status);
    CHECK(status == U_INVALID_CHAR_FOUND && target == output);
    status = U_ZERO_ERROR;
    ucnv_setToUCallBack_46(utf8, old_to, old_to_context,
                         &restored_to, &restored_context, &status);
    CHECK(restored_to == UCNV_TO_U_CALLBACK_STOP_46 && restored_context == &context);
    ucnv_setFromUCallBack_46(ascii, old_from, old_from_context,
                           &restored_from, &restored_context, &status);
    CHECK(restored_from == UCNV_FROM_U_CALLBACK_STOP_46 && restored_context == &context);
    CHECK(U_SUCCESS(status));
    ucnv_close_46(utf8); ucnv_close_46(ascii);
    return 0;
}

static int fd_count(void)
{
    DIR *dir = opendir("/proc/self/fd");
    struct dirent *entry;
    int count = 0;
    if (!dir) return -1;
    while ((entry = readdir(dir)))
        if (entry->d_name[0] != '.') ++count;
    closedir(dir);
    return count;
}

static int legacy_strings(void)
{
    const char16_t source[] = {'A', 0, 0x4e2d, 0xd83d, 0xde00};
    const char modified[] = "A\xc0\x80\xe4\xb8\xad\xed\xa0\xbd\xed\xb8\x80";
    const char standard[] = "A\xe4\xb8\xad\xf0\x9f\x98\x80";
    const char16_t standard16[] = {'A', 0x4e2d, 0xd83d, 0xde00};
    size_t length = 0;
    char *s8 = strndup16to8(source, sizeof(source) / sizeof(source[0]));
    char16_t *s16;
    CHECK(s8 && memcmp(s8, modified, sizeof(modified)) == 0);
    s16 = strdup8to16(s8, &length);
    CHECK(s16 && length == sizeof(source) / sizeof(source[0]));
    CHECK(memcmp(s16, source, sizeof(source)) == 0);
    free(s8); free(s16);
    s16 = strdup8to16(standard, &length);
    CHECK(s16 && length == sizeof(standard16) / sizeof(standard16[0]));
    CHECK(memcmp(s16, standard16, sizeof(standard16)) == 0);
    free(s16);
    s8 = strndup16to8(source, 1);
    CHECK(s8 && strcmp(s8, "A") == 0);
    free(s8);
    s8 = strndup16to8(source, 0);
    CHECK(s8 && s8[0] == 0);
    free(s8);
    s16 = strdup8to16("", &length);
    CHECK(s16 && length == 0);
    free(s16);
    CHECK(strndup16to8(NULL, 0) == NULL);
    CHECK(strdup8to16(NULL, &length) == NULL);
    return 0;
}

int main(void)
{
    const char utf8[] = "A\xe4\xb8\xad\xf0\x9f\x98\x80";
    const char utf16[] = {'A', 0, 0x2d, 0x4e, 0x3d, (char)0xd8, 0, (char)0xde};
    UErrorCode status = U_ZERO_ERROR;
    int mtu, expected_mtu, before, i;
    FILE *file;
    CHECK(legacy_strings() == 0);
    CHECK(convert("UTF-8", "UTF-16LE", utf8, sizeof(utf8)-1,
                  utf16, sizeof(utf16), 0) == 0);
    CHECK(convert("UTF-16LE", "UTF-8", utf16, sizeof(utf16),
                  utf8, sizeof(utf8)-1, 1) == 0);
    CHECK(callbacks() == 0);
    CHECK(ucnv_open_46("mione-not-a-charset", &status) == NULL && U_FAILURE(status));
    ucnv_close_46(NULL);
    file = fopen("/sys/class/net/lo/mtu", "r");
    CHECK(file && fscanf(file, "%d", &expected_mtu) == 1);
    fclose(file);
    CHECK(ifc_get_mtu("lo", &mtu) == 0 && mtu == expected_mtu);
    CHECK(ifc_get_mtu("lo", NULL) == -1);
    CHECK(ifc_get_mtu("mione-missing", &mtu) == -2 && mtu == 0);
    CHECK(ifc_get_mtu("name-too-long-for-an-interface", &mtu) == -2 && mtu == 0);
    CHECK(ifc_get_mtu(NULL, &mtu) == -2 && mtu == 0);
    before = fd_count();
    CHECK(before >= 0);
    for (i = 0; i < 100; ++i)
        CHECK(ifc_get_mtu("lo", &mtu) == 0 && mtu == expected_mtu);
    CHECK(fd_count() == before);
    printf("PASS: %d checks (legacy strings, ICU conversion, streaming, STOP callbacks, MTU, FD lifetime)\n", checks);
    return 0;
}

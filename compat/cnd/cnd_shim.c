/*
 * Copyright (C) 2026 The MiOne contributors
 * SPDX-License-Identifier: Apache-2.0
 */
#include "cnd_shim.h"

#include <errno.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <linux/sockios.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

/* CND imports these ICU 4.6 C APIs. Use Android's stable ICU C interface
 * for the matching functions and callback layouts. Converters and their data
 * remain owned by the runtime APEX. */
UConverter *ucnv_open_46(const char *name, UErrorCode *status)
{
    return ucnv_open(name, status);
}

void ucnv_close_46(UConverter *converter)
{
    ucnv_close(converter);
}

void ucnv_setToUCallBack_46(UConverter *converter, UConverterToUCallback action,
                          const void *context, UConverterToUCallback *old_action,
                          const void **old_context, UErrorCode *status)
{
    ucnv_setToUCallBack(converter, action, context, old_action, old_context, status);
}

void ucnv_setFromUCallBack_46(UConverter *converter, UConverterFromUCallback action,
                            const void *context, UConverterFromUCallback *old_action,
                            const void **old_context, UErrorCode *status)
{
    ucnv_setFromUCallBack(converter, action, context, old_action, old_context, status);
}

void ucnv_convertEx_46(UConverter *target_cnv, UConverter *source_cnv,
                      char **target, const char *target_limit,
                      const char **source, const char *source_limit,
                      UChar *pivot_start, UChar **pivot_source, UChar **pivot_target,
                      const UChar *pivot_limit, UBool reset, UBool flush,
                      UErrorCode *status)
{
    ucnv_convertEx(target_cnv, source_cnv, target, target_limit, source, source_limit,
                   pivot_start, pivot_source, pivot_target, pivot_limit,
                   reset, flush, status);
}

void UCNV_FROM_U_CALLBACK_STOP_46(const void *context,
                                UConverterFromUnicodeArgs *args,
                                const UChar *units, int32_t length,
                                UChar32 point, UConverterCallbackReason reason,
                                UErrorCode *status)
{
    UCNV_FROM_U_CALLBACK_STOP(context, args, units, length, point, reason, status);
}

void UCNV_TO_U_CALLBACK_STOP_46(const void *context,
                              UConverterToUnicodeArgs *args,
                              const char *units, int32_t length,
                              UConverterCallbackReason reason, UErrorCode *status)
{
    UCNV_TO_U_CALLBACK_STOP(context, args, units, length, reason, status);
}

/* CAF's old two-argument API returns 0 and fills the MTU, -1 for a missing
 * output pointer, or -2 (with MTU 0) on ioctl failure. Use our own socket so
 * concurrent calls cannot close or change libnetutils' private control socket. */
int ifc_get_mtu(const char *name, int *mtu)
{
    struct ifreq request;
    int fd, rc, saved_errno;

    if (!mtu) {
        errno = EINVAL;
        return -1;
    }
    *mtu = 0;
    if (!name || !*name) {
        errno = EINVAL;
        return -2;
    }
    if (strnlen(name, IFNAMSIZ) == IFNAMSIZ) {
        errno = ENAMETOOLONG;
        return -2;
    }
    memset(&request, 0, sizeof(request));
    strlcpy(request.ifr_name, name, sizeof(request.ifr_name));
    fd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if (fd < 0)
        return -2;
    rc = ioctl(fd, SIOCGIFMTU, &request);
    saved_errno = errno;
    close(fd);
    errno = saved_errno;
    if (rc < 0)
        return -2;
    *mtu = request.ifr_mtu;
    return 0;
}

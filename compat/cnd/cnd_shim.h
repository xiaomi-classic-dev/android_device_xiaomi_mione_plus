/* SPDX-License-Identifier: Apache-2.0 */
#ifndef MIONE_CND_SHIM_H
#define MIONE_CND_SHIM_H

#include <unicode/ucnv.h>
#include <unicode/ucnv_err.h>

UConverter *ucnv_open_46(const char *, UErrorCode *);
void ucnv_close_46(UConverter *);
void ucnv_setToUCallBack_46(UConverter *, UConverterToUCallback, const void *,
                          UConverterToUCallback *, const void **, UErrorCode *);
void ucnv_setFromUCallBack_46(UConverter *, UConverterFromUCallback, const void *,
                            UConverterFromUCallback *, const void **, UErrorCode *);
void ucnv_convertEx_46(UConverter *, UConverter *, char **, const char *,
                      const char **, const char *, UChar *, UChar **, UChar **,
                      const UChar *, UBool, UBool, UErrorCode *);
void UCNV_FROM_U_CALLBACK_STOP_46(const void *, UConverterFromUnicodeArgs *,
                                const UChar *, int32_t, UChar32,
                                UConverterCallbackReason, UErrorCode *);
void UCNV_TO_U_CALLBACK_STOP_46(const void *, UConverterToUnicodeArgs *,
                              const char *, int32_t,
                              UConverterCallbackReason, UErrorCode *);
int ifc_get_mtu(const char *, int *);

#endif

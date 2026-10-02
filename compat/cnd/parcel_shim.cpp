/* SPDX-License-Identifier: Apache-2.0 */
#include <binder/Parcel.h>
#include <stdint.h>

static_assert(sizeof(uint16_t) == sizeof(char16_t), "UTF-16 unit size differs");

extern "C" android::status_t mione_parcel_write_string16(
        android::Parcel *parcel, const uint16_t *string, size_t length)
        __asm__("_ZN7android6Parcel13writeString16EPKtj");

extern "C" android::status_t mione_parcel_write_string16(
        android::Parcel *parcel, const uint16_t *string, size_t length)
{
    return parcel->writeString16(reinterpret_cast<const char16_t *>(string), length);
}

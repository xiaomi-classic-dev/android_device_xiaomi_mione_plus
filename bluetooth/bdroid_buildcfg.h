/*
 * Copyright (C) 2012 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _BDROID_BUILDCFG_H
#define _BDROID_BUILDCFG_H

#define BTM_DEF_LOCAL_NAME   "Xiaomi MI-1"
#define BTA_DISABLE_DELAY 1000 /* in milliseconds */
#define BLE_INCLUDED FALSE
#define BTA_GATT_INCLUDED FALSE
#define SMP_INCLUDED FALSE
#define REMOVE_EAGER_THREADS FALSE
#define HCI_BCM4329_PATCH_DOWNLOAD_QUIRK TRUE

/* BCM4329B1 firmware 0x0321 cannot report BR/EDR encryption key size.
 * This explicitly accepts an unverified key size for that controller only;
 * weak-key/KNOB protection from the key-size query is unavailable here.
 */
#define BTM_ALLOW_UNVERIFIED_BR_EDR_KEY_SIZE(status, version) \
    ((status) == HCI_ERR_ILLEGAL_COMMAND && (version) != NULL && \
     (version)->hci_version == HCI_PROTO_VERSION_2_1 && \
     (version)->hci_revision == 0x0321 && \
     (version)->lmp_version == HCI_PROTO_VERSION_2_1 && \
     (version)->manufacturer == 15 && \
     (version)->lmp_subversion == 0x4217)

#endif

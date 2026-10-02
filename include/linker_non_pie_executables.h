/*
 * Copyright (C) 2026 The LineageOS Project
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

#ifndef MIONE_LINKER_NON_PIE_EXECUTABLES_H
#define MIONE_LINKER_NON_PIE_EXECUTABLES_H

// Exact installed paths of the legacy ET_EXEC vendor executables.
const char* linker_non_pie_executables[] = {
    "/system/bin/bridgemgrd",
    "/system/bin/cnd",
    "/system/bin/gpsone_daemon",
    "/system/bin/netmgrd",
    "/system/bin/port-bridge",
    "/system/bin/qmiproxy",
    "/system/bin/qmuxd",
    "/system/bin/rmt_storage",
    "/system/bin/usbhub",
    "/system/bin/usbhub_init",
};

#endif // MIONE_LINKER_NON_PIE_EXECUTABLES_H

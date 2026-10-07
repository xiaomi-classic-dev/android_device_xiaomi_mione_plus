/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#define LOG_TAG "mione-power-service"
#include <hidl/HidlTransportSupport.h>
#include <log/log.h>
#include "Power.h"

int main() {
    android::hardware::configureRpcThreadpool(1, true);
    android::sp<android::hardware::power::V1_0::IPower> service =
        new android::hardware::power::V1_0::implementation::Power();
    if (service->registerAsService() != android::OK) {
        ALOGE("Cannot register MiOne power service");
        return 1;
    }
    android::hardware::joinRpcThreadpool();
    return 1;
}

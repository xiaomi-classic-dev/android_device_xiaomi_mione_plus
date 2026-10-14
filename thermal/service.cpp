/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#define LOG_TAG "mione-thermal-service"
#include <hidl/HidlTransportSupport.h>
#include <log/log.h>
#include "Thermal.h"

int main() {
    android::hardware::configureRpcThreadpool(4, true);
    android::sp<android::hardware::thermal::V1_0::implementation::Thermal> service =
        new android::hardware::thermal::V1_0::implementation::Thermal();
    if (service->registerAsService() != android::OK) {
        ALOGE("Cannot register MiOne thermal service");
        return 1;
    }
    service->start();
    android::hardware::joinRpcThreadpool();
    return 1;
}

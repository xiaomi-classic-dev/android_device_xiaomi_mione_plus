/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#define LOG_TAG "mione-light-service"
#include <hidl/HidlTransportSupport.h>
#include <log/log.h>
#include "Light.h"

int main() {
    android::hardware::configureRpcThreadpool(1, true);
    android::sp<android::hardware::light::V2_0::ILight> service =
        new android::hardware::light::V2_0::implementation::Light();
    if (service->registerAsService() != android::OK) {
        ALOGE("Cannot register MiOne lights service");
        return 1;
    }
    android::hardware::joinRpcThreadpool();
    return 1;
}

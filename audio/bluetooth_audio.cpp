/*
 * Copyright (C) 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */

#define LOG_TAG "msm8660-btaudio"

#include <android/hardware/bluetooth/audio/2.0/IBluetoothAudioProvidersFactory.h>
#include <dlfcn.h>
#include <errno.h>
#include <hardware/audio.h>
#include <hidl/LegacySupport.h>
#include <log/log.h>
#include <mutex>

namespace {
using ::android::hardware::bluetooth::audio::V2_0::IBluetoothAudioProvidersFactory;
std::once_flag initializeOnce;
const hw_module_t* implementation = nullptr;
int initializeResult = -ENODEV;

int openBluetooth(const hw_module_t*, const char* name, hw_device_t** device) {
    std::call_once(initializeOnce, [] {
        // The provider and PCM module must share the audioserver session state.
        auto status = ::android::hardware::registerPassthroughServiceImplementation<
                IBluetoothAudioProvidersFactory>();
        if (status != ::android::OK) {
            ALOGE("Bluetooth audio provider registration failed: %d", status);
            return;
        }
        void* handle = dlopen("/vendor/lib/hw/audio.bluetooth.default.so", RTLD_NOW);
        if (handle == nullptr) {
            ALOGE("Bluetooth PCM implementation load failed: %s", dlerror());
            return;
        }
        implementation = static_cast<const hw_module_t*>(dlsym(handle, HAL_MODULE_INFO_SYM_AS_STR));
        if (implementation == nullptr || implementation->methods == nullptr ||
                implementation->methods->open == nullptr) {
            ALOGE("Bluetooth PCM implementation has no module entry");
            return;
        }
        initializeResult = 0;
    });
    if (initializeResult != 0) return initializeResult;
    return implementation->methods->open(implementation, name, device);
}

hw_module_methods_t methods = {.open = openBluetooth};
}  // namespace

extern "C" {
audio_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = AUDIO_MODULE_API_VERSION_0_1,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = AUDIO_HARDWARE_MODULE_ID,
        .name = "MSM8660 Bluetooth audio provider bridge",
        .author = "The LineageOS Project",
        .methods = &methods,
    },
};
}

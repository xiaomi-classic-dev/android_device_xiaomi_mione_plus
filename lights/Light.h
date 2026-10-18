/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <aidl/android/hardware/light/BnLights.h>
#include <vector>

namespace aidl::android::hardware::light::implementation {
using LightState = HwLightState;
using Type = LightType;
using Flash = FlashMode;
using Brightness = BrightnessMode;
class Light : public BnLights {
public:
    ndk::ScopedAStatus setLightState(int32_t id, const HwLightState& state) override;
    ndk::ScopedAStatus getLights(std::vector<HwLight>* lights) override;
};
}

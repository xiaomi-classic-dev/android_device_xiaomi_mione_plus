/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <android/hardware/light/2.0/ILight.h>

namespace android { namespace hardware { namespace light { namespace V2_0 { namespace implementation {
class Light : public ILight {
public:
    Return<Status> setLight(Type type, const LightState& state) override;
    Return<void> getSupportedTypes(getSupportedTypes_cb cb) override;
};
}}}}}

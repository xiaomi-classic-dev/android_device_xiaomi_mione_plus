/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <android/hardware/power/1.0/IPower.h>

namespace android { namespace hardware { namespace power { namespace V1_0 { namespace implementation {
class Power : public IPower {
public:
    Power();
    Return<void> setInteractive(bool interactive) override;
    Return<void> powerHint(PowerHint hint, int32_t data) override;
    Return<void> setFeature(Feature feature, bool activate) override;
    Return<void> getPlatformLowPowerStats(getPlatformLowPowerStats_cb cb) override;
};
}}}}}

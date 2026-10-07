/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <android/hardware/thermal/1.0/IThermal.h>

namespace android { namespace hardware { namespace thermal { namespace V1_0 { namespace implementation {
class Thermal : public IThermal {
public:
    Return<void> getTemperatures(getTemperatures_cb cb) override;
    Return<void> getCpuUsages(getCpuUsages_cb cb) override;
    Return<void> getCoolingDevices(getCoolingDevices_cb cb) override;
};
}}}}}

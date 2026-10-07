/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#include "Power.h"
#include "power_policy.h"

namespace android { namespace hardware { namespace power { namespace V1_0 { namespace implementation {
Power::Power() { mione_power_init(); }

Return<void> Power::setInteractive(bool interactive) {
    mione_power_set_interactive(interactive);
    return Void();
}

Return<void> Power::powerHint(PowerHint hint, int32_t /* data */) {
    if (hint == PowerHint::INTERACTION)
        mione_power_interaction();
    return Void();
}

Return<void> Power::setFeature(Feature /* feature */, bool /* activate */) {
    // MiOne has no double-tap-to-wake control.
    return Void();
}

Return<void> Power::getPlatformLowPowerStats(getPlatformLowPowerStats_cb cb) {
    // Keep the previous HAL's empty platform residency statistics.
    cb(hidl_vec<PowerStatePlatformSleepState>(), Status::SUCCESS);
    return Void();
}
}}}}}

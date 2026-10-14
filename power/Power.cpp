/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#include <aidl/android/hardware/power/BnPower.h>
#include <android/binder_manager.h>
#include <android/binder_process.h>
#include <unistd.h>
#include "power_policy.h"

using namespace aidl::android::hardware::power;
using ndk::ScopedAStatus;

class MionePower final : public BnPower {
public:
    MionePower() { mione_power_init(); }
    ScopedAStatus setMode(Mode mode, bool enabled) override {
        if (mode == Mode::INTERACTIVE)
            mione_power_set_interactive(enabled);
        else if (mode == Mode::LAUNCH && enabled)
            mione_power_interaction();
        return ScopedAStatus::ok();
    }
    ScopedAStatus isModeSupported(Mode mode, bool* supported) override {
        *supported = mode == Mode::INTERACTIVE || mode == Mode::LAUNCH;
        return ScopedAStatus::ok();
    }
    ScopedAStatus setBoost(Boost boost, int32_t durationMs) override {
        // MSM8660 has a fixed one-shot pulse without a duration control or
        // persistent boost to cancel with a negative hint.
        if (boost == Boost::INTERACTION && durationMs >= 0)
            mione_power_interaction();
        return ScopedAStatus::ok();
    }
    ScopedAStatus isBoostSupported(Boost boost, bool* supported) override {
        *supported = boost == Boost::INTERACTION &&
            access("/sys/devices/system/cpu/cpufreq/interactive/boostpulse", W_OK) == 0;
        return ScopedAStatus::ok();
    }
    ScopedAStatus createHintSession(int32_t, int32_t, const std::vector<int32_t>&,
            int64_t, std::shared_ptr<IPowerHintSession>* session) override {
        *session = nullptr;
        return ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
    ScopedAStatus getHintSessionPreferredRate(int64_t* rate) override {
        *rate = 0;
        return ScopedAStatus::fromExceptionCode(EX_UNSUPPORTED_OPERATION);
    }
};

int main() {
    ABinderProcess_setThreadPoolMaxThreadCount(1);
    auto service = ndk::SharedRefBase::make<MionePower>();
    std::string name = std::string(IPower::descriptor) + "/default";
    if (AServiceManager_addService(service->asBinder().get(), name.c_str()) != STATUS_OK)
        return 1;
    ABinderProcess_joinThreadPool();
    return 1;
}

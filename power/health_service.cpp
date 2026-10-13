/* Copyright (C) 2026 The CyanogenMod Project
 * SPDX-License-Identifier: Apache-2.0
 */
#include <health/utils.h>
#include <health2impl/BinderHealth.h>
#include <health2impl/Health.h>
#include <healthd/healthd.h>
#include <memory>
#include <utility>

using android::hardware::health::InitHealthdConfig;
using android::hardware::health::V2_1::HealthInfo;
using android::hardware::health::V2_1::implementation::BinderHealth;
using android::hardware::health::V2_1::implementation::Health;

class MioneHealth : public Health {
public:
    explicit MioneHealth(std::unique_ptr<healthd_config>&& config)
        : Health(std::move(config)) {}
protected:
    void UpdateHealthInfo(HealthInfo* info) override {
        // Retain the existing battery-low policy and its hysteresis.
        android::BatteryProperties props{};
        props.batteryPresent = info->legacy.legacy.batteryPresent;
        props.batteryLevel = info->legacy.legacy.batteryLevel;
        healthd_board_battery_update(&props);
    }
};

int main() {
    auto config = std::make_unique<healthd_config>();
    InitHealthdConfig(config.get());
    healthd_board_init(config.get());
    android::sp<MioneHealth> health = new MioneHealth(std::move(config));
    android::sp<BinderHealth> service = new BinderHealth("default", health);
    return service->StartLoop();
}

/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#include <android/hardware/light/2.0/ILight.h>
#include <android/hardware/power/1.0/IPower.h>
#include <android/hardware/thermal/1.0/IThermal.h>
#include <cmath>
#include <cstdio>
#include <set>

using android::hardware::hidl_vec;
namespace light = android::hardware::light::V2_0;
namespace power = android::hardware::power::V1_0;
namespace thermal = android::hardware::thermal::V1_0;
static int checks, failures;
#define CHECK(expr) do { ++checks; if (!(expr)) { ++failures; printf("FAIL line %d: %s\n", __LINE__, #expr); } } while (0)

static float readTemperature(const char* path, float divisor) {
    FILE* file = fopen(path, "r");
    float value = NAN;
    if (file) {
        if (fscanf(file, "%f", &value) != 1)
            value = NAN;
        fclose(file);
    }
    return value/divisor;
}

int main() {
    auto lights = light::ILight::tryGetService();
    auto powers = power::IPower::tryGetService();
    auto thermals = thermal::IThermal::tryGetService();
    CHECK(lights != nullptr && powers != nullptr && thermals != nullptr);
    if (failures)
        return 1;
    CHECK(lights->isRemote() && powers->isRemote() && thermals->isRemote());
    bool called = false;
    std::set<light::Type> supported;
    CHECK(lights->getSupportedTypes([&](const hidl_vec<light::Type>& types) {
        called = true;
        for (auto type : types)
            supported.insert(type);
    }).isOk());
    CHECK(called && supported.size() == 4);
    CHECK(supported.count(light::Type::BACKLIGHT) && supported.count(light::Type::BUTTONS) &&
          supported.count(light::Type::BATTERY) && supported.count(light::Type::NOTIFICATIONS));
    light::LightState state{};
    state.color = 0xff404040;
    state.brightnessMode = light::Brightness::LOW_PERSISTENCE;
    auto low = lights->setLight(light::Type::BACKLIGHT, state);
    CHECK(low.isOk() && static_cast<light::Status>(low) == light::Status::BRIGHTNESS_NOT_SUPPORTED);
    state.brightnessMode = light::Brightness::USER;
    auto unsupported = lights->setLight(light::Type::KEYBOARD, state);
    CHECK(unsupported.isOk() && static_cast<light::Status>(unsupported) == light::Status::LIGHT_NOT_SUPPORTED);
    called = false;
    CHECK(powers->getPlatformLowPowerStats([&](const hidl_vec<power::PowerStatePlatformSleepState>& states, power::Status status) {
        called = true;
        CHECK(status == power::Status::SUCCESS);
        printf("Power platform states: %zu\n", states.size());
    }).isOk());
    CHECK(called);
    for (int iteration = 0; iteration < 3; ++iteration) {
        float tsens = readTemperature("/sys/class/thermal/thermal_zone0/temp", 1);
        float battery = readTemperature("/sys/class/power_supply/battery/temp", 10);
        called = false;
        CHECK(thermals->getTemperatures([&](const thermal::ThermalStatus& status, const hidl_vec<thermal::Temperature>& temperatures) {
            called = true;
            CHECK(status.code == thermal::ThermalStatusCode::SUCCESS);
            CHECK(temperatures.size() == 2);
            for (auto& value : temperatures) {
                CHECK(std::isfinite(value.currentValue) && std::abs(value.currentValue) < 200);
                printf("Temperature %s: %.1f C throttle=%.1f shutdown=%.1f\n", value.name.c_str(), value.currentValue, value.throttlingThreshold, value.shutdownThreshold);
                if (value.type == thermal::TemperatureType::CPU)
                    CHECK(std::abs(value.currentValue-tsens) <= 5);
                if (value.type == thermal::TemperatureType::BATTERY)
                    CHECK(std::abs(value.currentValue-battery) <= 5);
            }
        }).isOk());
        CHECK(called);
        called = false;
        CHECK(thermals->getCpuUsages([&](const thermal::ThermalStatus& status, const hidl_vec<thermal::CpuUsage>& cpus) {
            called = true;
            CHECK(status.code == thermal::ThermalStatusCode::SUCCESS);
            CHECK(cpus.size() == 2);
            for (auto& cpu : cpus) {
                CHECK(!cpu.isOnline || cpu.active <= cpu.total);
                printf("CPU %s online=%d active=%llu total=%llu ms\n", cpu.name.c_str(), cpu.isOnline, static_cast<unsigned long long>(cpu.active), static_cast<unsigned long long>(cpu.total));
            }
        }).isOk());
        CHECK(called);
    }
    called = false;
    CHECK(thermals->getCoolingDevices([&](const thermal::ThermalStatus& status, const hidl_vec<thermal::CoolingDevice>& devices) {
        called = true;
        CHECK(status.code == thermal::ThermalStatusCode::SUCCESS);
        CHECK(devices.size() == 0);
    }).isOk());
    CHECK(called);
    printf("MiOne HIDL: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}

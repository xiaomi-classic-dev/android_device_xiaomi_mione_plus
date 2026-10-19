/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#include <aidl/android/hardware/light/ILights.h>
#include <aidl/android/hardware/power/IPower.h>
#include <android/binder_ibinder.h>
#include <android/binder_manager.h>
#include <android/hardware/thermal/1.0/IThermal.h>
#include <cmath>
#include <cstdio>
#include <map>
#include <memory>
#include <string>
#include <vector>

using android::hardware::hidl_vec;
namespace light = aidl::android::hardware::light;
namespace power = aidl::android::hardware::power;
namespace thermal = android::hardware::thermal::V1_0;
static int checks, failures;
#define CHECK(expr) do { ++checks; if (!(expr)) { ++failures; printf("FAIL line %d: %s\n", __LINE__, #expr); } } while (0)

template <typename Interface>
static std::shared_ptr<Interface> getAidlService() {
    const std::string name = std::string(Interface::descriptor) + "/default";
    return Interface::fromBinder(ndk::SpAIBinder(AServiceManager_checkService(name.c_str())));
}

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
    auto lights = getAidlService<light::ILights>();
    auto powers = getAidlService<power::IPower>();
    auto thermals = thermal::IThermal::tryGetService();
    CHECK(lights != nullptr && powers != nullptr && thermals != nullptr);
    if (failures)
        return 1;
    CHECK(AIBinder_isRemote(lights->asBinder().get()) &&
          AIBinder_isRemote(powers->asBinder().get()) && thermals->isRemote());
    std::vector<light::HwLight> descriptions;
    CHECK(lights->getLights(&descriptions).isOk());
    std::map<light::LightType, int32_t> supported;
    for (const auto& description : descriptions)
        CHECK(supported.emplace(description.type, description.id).second);
    CHECK(supported.size() == 4);
    CHECK(supported.count(light::LightType::BACKLIGHT) && supported.count(light::LightType::BUTTONS) &&
          supported.count(light::LightType::BATTERY) && supported.count(light::LightType::NOTIFICATIONS));
    if (failures)
        return 1;
    light::HwLightState state{};
    state.color = static_cast<int32_t>(0xff404040u);
    state.brightnessMode = light::BrightnessMode::LOW_PERSISTENCE;
    auto low = lights->setLightState(supported.at(light::LightType::BACKLIGHT), state);
    CHECK(!low.isOk() && low.getExceptionCode() == EX_UNSUPPORTED_OPERATION);
    state.brightnessMode = light::BrightnessMode::USER;
    auto unsupported = lights->setLightState(-1, state);
    CHECK(!unsupported.isOk() && unsupported.getExceptionCode() == EX_UNSUPPORTED_OPERATION);
    bool modeSupported = false;
    CHECK(powers->isModeSupported(power::Mode::INTERACTIVE, &modeSupported).isOk());
    CHECK(modeSupported);
    CHECK(powers->isModeSupported(power::Mode::LAUNCH, &modeSupported).isOk());
    CHECK(modeSupported);
    CHECK(powers->isModeSupported(power::Mode::LOW_POWER, &modeSupported).isOk());
    CHECK(!modeSupported);
    int64_t preferredRate = 0;
    auto hintRate = powers->getHintSessionPreferredRate(&preferredRate);
    CHECK(!hintRate.isOk() && hintRate.getExceptionCode() == EX_UNSUPPORTED_OPERATION);
    bool called = false;
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
    printf("MiOne AIDL/HIDL: %d checks, %d failures\n", checks, failures);
    return failures != 0;
}

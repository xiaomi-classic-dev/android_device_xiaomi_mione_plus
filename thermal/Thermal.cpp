/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#include "Thermal.h"
#include <android-base/file.h>
#include <android-base/strings.h>
#include <cmath>
#include <fstream>
#include <sstream>
#include <string>
#include <unistd.h>

namespace android { namespace hardware { namespace thermal { namespace V1_0 { namespace implementation {
namespace {
const char kTsens[] = "/sys/class/thermal/thermal_zone0/";

bool readNumber(const std::string& path, double* value) {
    std::ifstream stream(path);
    if (!(stream >> *value) || !std::isfinite(*value))
        return false;
    stream >> std::ws;
    return stream.eof();
}

float shutdownThreshold() {
    std::string type;
    double value;
    if (!android::base::ReadFileToString(std::string(kTsens)+"trip_point_0_type", &type) ||
        android::base::Trim(type) != "critical" ||
        !readNumber(std::string(kTsens)+"trip_point_0_temp", &value))
        return NAN;
    return static_cast<float>(value);
}

float throttlingThreshold() {
    // Report the active thermald policy; the HIDL service never programs it.
    std::ifstream config("/vendor/etc/thermald.conf");
    std::string line;
    bool section = false;
    while (std::getline(config, line)) {
        line = android::base::Trim(line);
        if (!line.empty() && line[0] == '[')
            section = line == "[tsens_tz_sensor0]";
        if (!section)
            continue;
        std::istringstream values(line);
        std::string key;
        float value;
        if (values >> key >> value && key == "thresholds" && std::isfinite(value))
            return value;
    }
    return NAN;
}

uint64_t toMilliseconds(uint64_t ticks, uint64_t hz) {
    return (ticks/hz)*1000 + (ticks%hz)*1000/hz;
}
}  // namespace

Return<void> Thermal::getTemperatures(getTemperatures_cb cb) {
    ThermalStatus status;
    status.code = ThermalStatusCode::SUCCESS;
    hidl_vec<Temperature> temperatures;
    temperatures.resize(2);
    for (auto& value : temperatures) {
        value.currentValue = NAN;
        value.throttlingThreshold = NAN;
        value.shutdownThreshold = NAN;
        value.vrThrottlingThreshold = NAN;
    }
    temperatures[0].type = TemperatureType::CPU;
    temperatures[0].name = "tsens_tz_sensor0";
    temperatures[1].type = TemperatureType::BATTERY;
    temperatures[1].name = "battery";
    std::string type;
    double value;
    // MSM8660's legacy TSENS driver returns whole Celsius, not millidegrees.
    bool tsens = android::base::ReadFileToString(std::string(kTsens)+"type", &type) &&
        android::base::Trim(type) == "tsens_tz_sensor0" &&
        readNumber(std::string(kTsens)+"temp", &value);
    if (tsens) {
        temperatures[0].currentValue = static_cast<float>(value);
        temperatures[0].throttlingThreshold = throttlingThreshold();
        temperatures[0].shutdownThreshold = shutdownThreshold();
    }
    // power_supply temperature uses the standard tenths of Celsius unit.
    bool battery = readNumber("/sys/class/power_supply/battery/temp", &value);
    if (battery)
        temperatures[1].currentValue = static_cast<float>(value/10.0);
    if (!tsens || !battery) {
        status.code = ThermalStatusCode::FAILURE;
        status.debugMessage = !tsens ? "Cannot read MiOne TSENS temperature" :
                                     "Cannot read MiOne battery temperature";
    }
    cb(status, temperatures);
    return Void();
}

Return<void> Thermal::getCpuUsages(getCpuUsages_cb cb) {
    ThermalStatus status;
    status.code = ThermalStatusCode::SUCCESS;
    hidl_vec<CpuUsage> usages;
    // MiOne has two possible CPUs. Preserve both entries while CPU1 is offline.
    usages.resize(2);
    bool found[2] = {false, false};
    bool valid = true;
    long hz = sysconf(_SC_CLK_TCK);
    for (unsigned int cpu = 0; cpu < 2; ++cpu) {
        usages[cpu].name = "cpu"+std::to_string(cpu);
        double online;
        if (!readNumber("/sys/devices/system/cpu/cpu"+std::to_string(cpu)+"/online", &online)) {
            usages[cpu].isOnline = false;
            valid = false;
        } else {
            usages[cpu].isOnline = online != 0;
        }
        usages[cpu].active = usages[cpu].total = 0;
    }
    std::ifstream stat("/proc/stat");
    if (!stat || hz <= 0)
        valid = false;
    std::string line;
    while (std::getline(stat, line) && hz > 0) {
        std::istringstream fields(line);
        std::string name;
        uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
        if (!(fields >> name) || (name != "cpu0" && name != "cpu1"))
            continue;
        unsigned int cpu = name == "cpu0" ? 0 : 1;
        if (!(fields >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal))
            continue;
        // guest/guest_nice are already included in user/nice; never add twice.
        uint64_t active = user+nice+system+irq+softirq+steal;
        usages[cpu].active = toMilliseconds(active, hz);
        usages[cpu].total = toMilliseconds(active+idle+iowait, hz);
        found[cpu] = true;
    }
    for (unsigned int cpu = 0; cpu < 2; ++cpu)
        if (usages[cpu].isOnline && !found[cpu])
            valid = false;
    if (!valid) {
        status.code = ThermalStatusCode::FAILURE;
        status.debugMessage = "Cannot read MiOne CPU online state or /proc/stat";
    }
    cb(status, usages);
    return Void();
}

Return<void> Thermal::getCoolingDevices(getCoolingDevices_cb cb) {
    // Thermal 1.0 only defines FAN_RPM. CPU frequency caps are not fans.
    ThermalStatus status;
    status.code = ThermalStatusCode::SUCCESS;
    cb(status, hidl_vec<CoolingDevice>());
    return Void();
}
}}}}}

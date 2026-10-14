/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#include "Thermal.h"
#include <android-base/file.h>
#include <android-base/strings.h>
#include <algorithm>
#include <chrono>
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

Thermal::Thermal() {
    for (auto& threshold : mThresholds) {
        for (size_t level = 0; level < 7; ++level) {
            threshold.hotThrottlingThresholds[level] = NAN;
            threshold.coldThrottlingThresholds[level] = NAN;
        }
        threshold.vrThrottlingThreshold = NAN;
    }
    mThresholds[0].type = t2::TemperatureType::CPU;
    mThresholds[0].name = "tsens_tz_sensor0";
    mThresholds[1].type = t2::TemperatureType::BATTERY;
    mThresholds[1].name = "battery";
    // Read the three actual MSM8660 thermald mitigation levels and hysteresis.
    // No APQ8064 channels or invented critical/emergency thresholds.
    std::ifstream config("/vendor/etc/thermald.conf");
    std::string line;
    bool section = false, hot = false, cold = false;
    while (std::getline(config, line)) {
        line = android::base::Trim(line);
        if (!line.empty() && line[0] == '[')
            section = line == "[tsens_tz_sensor0]";
        if (!section)
            continue;
        std::istringstream fields(line);
        std::string key;
        fields >> key;
        if (key != "thresholds" && key != "thresholds_clr")
            continue;
        auto& values = key == "thresholds" ? mThresholds[0].hotThrottlingThresholds :
                                            mThresholds[0].coldThrottlingThresholds;
        bool valid = true;
        for (size_t level = 1; level <= 3; ++level)
            valid = static_cast<bool>(fields >> values[level]) &&
                    std::isfinite(values[level]) && valid;
        if (key == "thresholds") hot = valid;
        else cold = valid;
    }
    mPolicyValid = hot && cold;
    for (size_t level = 1; level <= 3; ++level) {
        const auto& threshold = mThresholds[0];
        mPolicyValid = mPolicyValid &&
            threshold.coldThrottlingThresholds[level] < threshold.hotThrottlingThresholds[level];
        if (level > 1)
            mPolicyValid = mPolicyValid &&
                threshold.hotThrottlingThresholds[level-1] < threshold.hotThrottlingThresholds[level] &&
                threshold.coldThrottlingThresholds[level-1] < threshold.coldThrottlingThresholds[level];
    }
    const float shutdown = shutdownThreshold();
    mThresholds[0].hotThrottlingThresholds[6] = shutdown;
    mThresholds[0].coldThrottlingThresholds[6] = shutdown;
}

Thermal::~Thermal() {
    { std::lock_guard<std::mutex> guard(mClientMutex); mStopping = true; }
    mChanged.notify_all();
    if (mWorker.joinable()) mWorker.join();
}

void Thermal::start() { mWorker = std::thread([this] { poll(); }); }

Return<void> Thermal::getCurrentTemperatures(bool filter, t2::TemperatureType type,
        getCurrentTemperatures_cb cb) {
    std::vector<t2::Temperature> selected;
    bool valid = true;
    {
        std::lock_guard<std::mutex> guard(mSensorMutex);
        getTemperatures([&](const ThermalStatus&, const hidl_vec<Temperature>& legacy) {
            for (const auto& sensor : legacy) {
                auto severity = t2::ThrottlingSeverity::NONE;
                const auto sensorType = static_cast<t2::TemperatureType>(sensor.type);
                if (sensorType == t2::TemperatureType::CPU && std::isfinite(sensor.currentValue)) {
                    if (mPolicyValid) {
                        while (mCpuSeverity > 0 && sensor.currentValue <=
                                mThresholds[0].coldThrottlingThresholds[mCpuSeverity]) --mCpuSeverity;
                        while (mCpuSeverity < 3 && sensor.currentValue >=
                                mThresholds[0].hotThrottlingThresholds[mCpuSeverity+1]) ++mCpuSeverity;
                        severity = static_cast<t2::ThrottlingSeverity>(mCpuSeverity);
                    }
                    const float shutdown = mThresholds[0].hotThrottlingThresholds[6];
                    if (std::isfinite(shutdown) && sensor.currentValue >= shutdown)
                        severity = t2::ThrottlingSeverity::SHUTDOWN;
                }
                if (!filter || sensorType == type) {
                    valid = valid && std::isfinite(sensor.currentValue) &&
                        (sensorType != t2::TemperatureType::CPU || mPolicyValid);
                    selected.push_back({sensorType, sensor.name, sensor.currentValue, severity});
                }
            }
        });
    }
    valid = valid && !selected.empty();
    cb({valid ? ThermalStatusCode::SUCCESS : ThermalStatusCode::FAILURE,
        valid ? "" : "MiOne sensor or thermald policy is unavailable"}, selected);
    return Void();
}

Return<void> Thermal::getTemperatureThresholds(bool filter, t2::TemperatureType type,
        getTemperatureThresholds_cb cb) {
    std::vector<t2::TemperatureThreshold> selected;
    bool valid = true;
    for (const auto& threshold : mThresholds) {
        if (filter && threshold.type != type) continue;
        if (threshold.type == t2::TemperatureType::CPU) valid = mPolicyValid;
        selected.push_back(threshold);
    }
    valid = valid && !selected.empty();
    cb({valid ? ThermalStatusCode::SUCCESS : ThermalStatusCode::FAILURE,
        valid ? "" : "Requested MiOne thermal policy is unavailable"}, selected);
    return Void();
}

Return<void> Thermal::getCurrentCoolingDevices(bool, t2::CoolingType,
        getCurrentCoolingDevices_cb cb) {
    // The kernel has no cooling_device state interface; thermald sets CPU caps.
    cb({ThermalStatusCode::SUCCESS, ""}, hidl_vec<t2::CoolingDevice>());
    return Void();
}

Return<void> Thermal::registerThermalChangedCallback(const sp<t2::IThermalChangedCallback>& callback,
        bool filter, t2::TemperatureType type, registerThermalChangedCallback_cb cb) {
    bool valid = callback != nullptr;
    {
        std::lock_guard<std::mutex> guard(mClientMutex);
        for (const auto& client : mClients)
            if (android::hardware::interfacesEqual(client.callback, callback)) valid = false;
        if (valid) mClients.push_back({callback, filter, type, {{-1, -1}}});
    }
    if (valid) mChanged.notify_all();
    cb({valid ? ThermalStatusCode::SUCCESS : ThermalStatusCode::FAILURE,
        valid ? "" : "Null or already registered callback"});
    return Void();
}

Return<void> Thermal::unregisterThermalChangedCallback(const sp<t2::IThermalChangedCallback>& callback,
        unregisterThermalChangedCallback_cb cb) {
    bool removed;
    {
        std::lock_guard<std::mutex> guard(mClientMutex);
        const auto before = mClients.size();
        mClients.erase(std::remove_if(mClients.begin(), mClients.end(), [&](const Client& client) {
            return android::hardware::interfacesEqual(client.callback, callback);
        }), mClients.end());
        removed = mClients.size() != before;
    }
    mChanged.notify_all();
    cb({removed ? ThermalStatusCode::SUCCESS : ThermalStatusCode::FAILURE,
        removed ? "" : "Callback is not registered"});
    return Void();
}

void Thermal::poll() {
    for (;;) {
        {
            std::unique_lock<std::mutex> guard(mClientMutex);
            mChanged.wait(guard, [this] { return mStopping || !mClients.empty(); });
            if (mStopping) return;
        }
        std::vector<t2::Temperature> temperatures;
        getCurrentTemperatures(false, t2::TemperatureType::UNKNOWN,
            [&](const ThermalStatus&, const hidl_vec<t2::Temperature>& current) {
                temperatures.assign(current.begin(), current.end());
            });
        std::vector<std::pair<sp<t2::IThermalChangedCallback>, t2::Temperature>> notifications;
        {
            std::lock_guard<std::mutex> guard(mClientMutex);
            for (auto& client : mClients) for (size_t i = 0; i < temperatures.size(); ++i) {
                const auto& temperature = temperatures[i];
                if (!std::isfinite(temperature.value) || (client.filter && client.type != temperature.type))
                    continue;
                const int severity = static_cast<int>(temperature.throttlingStatus);
                if (client.last[i] != severity) {
                    client.last[i] = severity;
                    notifications.emplace_back(client.callback, temperature);
                }
            }
        }
        for (const auto& event : notifications) {
            const auto result = event.first->notifyThrottling(event.second);
            if (!result.isOk()) {
                std::lock_guard<std::mutex> guard(mClientMutex);
                mClients.erase(std::remove_if(mClients.begin(), mClients.end(), [&](const Client& client) {
                    return android::hardware::interfacesEqual(client.callback, event.first);
                }), mClients.end());
            }
        }
        std::unique_lock<std::mutex> guard(mClientMutex);
        mChanged.wait_for(guard, std::chrono::seconds(2), [this] { return mStopping || mClients.empty(); });
        if (mStopping) return;
    }
}
}}}}}

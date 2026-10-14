/* Copyright (C) 2026 The Xiaomi Classic Device Project
 * SPDX-License-Identifier: Apache-2.0
 */
#pragma once
#include <android/hardware/thermal/2.0/IThermal.h>
#include <array>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <vector>

namespace android { namespace hardware { namespace thermal { namespace V1_0 { namespace implementation {
namespace t2 = ::android::hardware::thermal::V2_0;
class Thermal : public t2::IThermal {
public:
    Thermal();
    ~Thermal() override;
    void start();
    Return<void> getTemperatures(getTemperatures_cb cb) override;
    Return<void> getCpuUsages(getCpuUsages_cb cb) override;
    Return<void> getCoolingDevices(getCoolingDevices_cb cb) override;
    Return<void> getCurrentTemperatures(bool filter, t2::TemperatureType type,
            getCurrentTemperatures_cb cb) override;
    Return<void> getTemperatureThresholds(bool filter, t2::TemperatureType type,
            getTemperatureThresholds_cb cb) override;
    Return<void> getCurrentCoolingDevices(bool filter, t2::CoolingType type,
            getCurrentCoolingDevices_cb cb) override;
    Return<void> registerThermalChangedCallback(const sp<t2::IThermalChangedCallback>& callback,
            bool filter, t2::TemperatureType type, registerThermalChangedCallback_cb cb) override;
    Return<void> unregisterThermalChangedCallback(const sp<t2::IThermalChangedCallback>& callback,
            unregisterThermalChangedCallback_cb cb) override;
private:
    struct Client {
        sp<t2::IThermalChangedCallback> callback;
        bool filter;
        t2::TemperatureType type;
        std::array<int, 2> last{{-1, -1}};
    };
    void poll();
    std::array<t2::TemperatureThreshold, 2> mThresholds;
    bool mPolicyValid = false;
    std::mutex mSensorMutex;
    int mCpuSeverity = 0;
    std::mutex mClientMutex;
    std::condition_variable mChanged;
    std::vector<Client> mClients;
    bool mStopping = false;
    std::thread mWorker;
};
}}}}}

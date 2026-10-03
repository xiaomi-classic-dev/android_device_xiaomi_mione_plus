/* Copyright (C) 2026 The LineageOS Project
 * SPDX-License-Identifier: Apache-2.0
 */
#define LOG_TAG "MiOneSensors"
#include <hardware/sensors.h>
#include <log/log.h>
#include <dlfcn.h>
#include <errno.h>
#include <pthread.h>
#include <string.h>
#include <map>
#include <vector>

static pthread_once_t module_once = PTHREAD_ONCE_INIT;
static sensors_module_t* vendor_module;
static std::vector<sensor_t> sensors;

static void load_module() {
    void* library = dlopen("/system/lib/hw/sensors.vendor.msm8660.so", RTLD_NOW | RTLD_LOCAL);
    if (!library) {
        ALOGE("Cannot load sensor HAL: %s", dlerror());
        return;
    }
    vendor_module = static_cast<sensors_module_t*>(dlsym(library, HAL_MODULE_INFO_SYM_AS_STR));
    if (!vendor_module || !vendor_module->get_sensors_list || !vendor_module->common.methods || !vendor_module->common.methods->open) {
        vendor_module = nullptr;
        return;
    }
    const sensor_t* list = nullptr;
    int count = vendor_module->get_sensors_list(vendor_module, &list);
    if (count <= 0 || !list) {
        vendor_module = nullptr;
        return;
    }
    sensors.assign(list, list + count);
    // The 0.1 sensor descriptor reserves the words that newer versions use.
    // Initialize those fields explicitly rather than trusting old reserved data.
    for (sensor_t& sensor : sensors) {
        sensor.fifoReservedEventCount = sensor.fifoMaxEventCount = 0;
        sensor.requiredPermission = nullptr;
        sensor.maxDelay = 1000000;
        sensor.flags = SENSOR_FLAG_CONTINUOUS_MODE;
        switch (sensor.type) {
        case SENSOR_TYPE_ACCELEROMETER:
            sensor.stringType = SENSOR_STRING_TYPE_ACCELEROMETER;
            break;
        case SENSOR_TYPE_MAGNETIC_FIELD:
            sensor.stringType = SENSOR_STRING_TYPE_MAGNETIC_FIELD;
            break;
        case SENSOR_TYPE_GYROSCOPE:
            sensor.stringType = SENSOR_STRING_TYPE_GYROSCOPE;
            break;
        case SENSOR_TYPE_LIGHT:
            sensor.stringType = SENSOR_STRING_TYPE_LIGHT;
            sensor.flags = SENSOR_FLAG_ON_CHANGE_MODE;
            break;
        case SENSOR_TYPE_PROXIMITY:
            sensor.stringType = SENSOR_STRING_TYPE_PROXIMITY;
            sensor.flags = SENSOR_FLAG_ON_CHANGE_MODE | SENSOR_FLAG_WAKE_UP;
            break;
        default:
            sensor.stringType = "";
        }
        memset(sensor.reserved, 0, sizeof(sensor.reserved));
    }
}

static const sensor_t* find_sensor(int handle) {
    for (const sensor_t& sensor : sensors)
        if (sensor.handle == handle) return &sensor;
    return nullptr;
}

struct Device {
    sensors_poll_device_1_t proxy;
    sensors_poll_device_t* vendor;
    pthread_mutex_t lock;
    std::map<int, int64_t> periods;
    std::map<int, bool> enabled;
};

static int activate(sensors_poll_device_t* dev, int handle, int enabled) {
    Device* ctx = reinterpret_cast<Device*>(dev);
    if (!find_sensor(handle)) return -EINVAL;
    pthread_mutex_lock(&ctx->lock);
    int result = ctx->vendor->activate(ctx->vendor, handle, enabled);
    if (result == 0) {
        ctx->enabled[handle] = enabled != 0;
        if (enabled && ctx->periods.count(handle)) {
            result = ctx->vendor->setDelay(ctx->vendor, handle, ctx->periods[handle]);
            if (result != 0 && ctx->vendor->activate(ctx->vendor, handle, 0) == 0)
                ctx->enabled[handle] = false;
        }
    }
    pthread_mutex_unlock(&ctx->lock);
    return result;
}

static int set_delay(sensors_poll_device_t* dev, int handle, int64_t period) {
    Device* ctx = reinterpret_cast<Device*>(dev);
    const sensor_t* sensor = find_sensor(handle);
    if (!sensor || period < 0) return -EINVAL;
    const int64_t minimum = static_cast<int64_t>(sensor->minDelay) * 1000;
    if (minimum > 0 && period < minimum) period = minimum;
    const int64_t maximum = static_cast<int64_t>(sensor->maxDelay) * 1000;
    if (maximum > 0 && period > maximum) period = maximum;
    pthread_mutex_lock(&ctx->lock);
    int result = 0;
    if (ctx->enabled[handle]) result = ctx->vendor->setDelay(ctx->vendor, handle, period);
    if (result == 0) ctx->periods[handle] = period;
    pthread_mutex_unlock(&ctx->lock);
    return result;
}

static int poll_events(sensors_poll_device_t* dev, sensors_event_t* data, int count) {
    Device* ctx = reinterpret_cast<Device*>(dev);
    if (!data || count <= 0) return -EINVAL;
    return ctx->vendor->poll(ctx->vendor, data, count);
}

static int batch(sensors_poll_device_1_t* dev, int handle, int flags,
                 int64_t period, int64_t latency) {
    if (flags != 0 || latency < 0) return -EINVAL;
    // No FIFO is advertised: deliver samples immediately through legacy poll.
    return set_delay(reinterpret_cast<sensors_poll_device_t*>(dev), handle, period);
}

static int flush(sensors_poll_device_1_t*, int) {
    // The legacy HAL has no flush operation; retain its explicit unsupported result.
    return -EINVAL;
}

static int close_device(hw_device_t* dev) {
    Device* ctx = reinterpret_cast<Device*>(dev);
    int result = ctx->vendor->common.close(&ctx->vendor->common);
    pthread_mutex_destroy(&ctx->lock);
    delete ctx;
    return result;
}

static int open_device(const hw_module_t* module, const char* name, hw_device_t** out) {
    if (!name || !out || strcmp(name, SENSORS_HARDWARE_POLL)) return -EINVAL;
    *out = nullptr;
    pthread_once(&module_once, load_module);
    if (!vendor_module) return -ENODEV;
    hw_device_t* vendor = nullptr;
    int result = vendor_module->common.methods->open(&vendor_module->common, name, &vendor);
    if (result != 0) return result;
    if (!vendor) return -ENODEV;
    sensors_poll_device_t* poll_device = reinterpret_cast<sensors_poll_device_t*>(vendor);
    if (!vendor->close || !poll_device->activate || !poll_device->setDelay || !poll_device->poll) {
        if (vendor->close) vendor->close(vendor);
        return -ENODEV;
    }
    Device* ctx = new Device();
    ctx->vendor = poll_device;
    pthread_mutex_init(&ctx->lock, nullptr);
    ctx->proxy.common.tag = HARDWARE_DEVICE_TAG;
    ctx->proxy.common.version = SENSORS_DEVICE_API_VERSION_1_3;
    ctx->proxy.common.module = const_cast<hw_module_t*>(module);
    ctx->proxy.common.close = close_device;
    ctx->proxy.activate = activate;
    ctx->proxy.setDelay = set_delay;
    ctx->proxy.poll = poll_events;
    ctx->proxy.batch = batch;
    ctx->proxy.flush = flush;
    *out = &ctx->proxy.common;
    return 0;
}

static int get_list(sensors_module_t*, const sensor_t** list) {
    if (!list) return -EINVAL;
    pthread_once(&module_once, load_module);
    *list = sensors.empty() ? nullptr : sensors.data();
    return static_cast<int>(sensors.size());
}

static hw_module_methods_t methods = {open_device};
extern "C" {
sensors_module_t HAL_MODULE_INFO_SYM = {
    {HARDWARE_MODULE_TAG, SENSORS_MODULE_API_VERSION_0_1, HARDWARE_HAL_API_VERSION,
     SENSORS_HARDWARE_MODULE_ID, "MiOne legacy sensor adapter", "LineageOS", &methods,
     nullptr, {0}},
    get_list, nullptr
};
}

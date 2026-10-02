/*
 * Copyright (C) 2026 The CyanogenMod Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#define LOG_TAG "MionePowerHAL"

#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <hardware/power.h>
#include "battery_policy.h"

#define _REALLY_INCLUDE_SYS__SYSTEM_PROPERTIES_H_
#include <sys/_system_properties.h>

#define CPU_COUNT 2
#define SCREEN_OFF_MAX_KHZ 918000U
#define BOOST_INTERVAL_NS 100000000LL
#define BOOSTPULSE_PATH "/sys/devices/system/cpu/cpufreq/interactive/boostpulse"

struct cpu_limit {
    unsigned int restore_khz;
    unsigned int applied_khz;
};

static struct cpu_limit limits[CPU_COUNT];
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static int interactive = 1;
static int battery_low;
static pthread_once_t watcher_once = PTHREAD_ONCE_INIT;
static int64_t last_boost_ns;

/* An offline CPU can lose its cpufreq directory; leave hotplug to the kernel. */
static unsigned int read_frequency(unsigned int cpu, const char *attribute)
{
    char path[128];
    unsigned int frequency = 0;
    FILE *file;

    snprintf(path, sizeof(path), "/sys/devices/system/cpu/cpu%u/cpufreq/%s",
             cpu, attribute);
    file = fopen(path, "r");
    if (file != NULL) {
        if (fscanf(file, "%u", &frequency) != 1)
            frequency = 0;
        fclose(file);
    }
    return frequency;
}

static int write_value(const char *path, const char *value)
{
    size_t length = strlen(value);
    ssize_t written;
    int fd;
    int error;

    do {
        fd = open(path, O_WRONLY | O_CLOEXEC);
    } while (fd < 0 && errno == EINTR);
    if (fd < 0) {
        error = errno;
        if (error != ENOENT && error != ENODEV)
            ALOGW("Cannot open %s: %s", path, strerror(error));
        return -error;
    }
    do {
        written = write(fd, value, length);
    } while (written < 0 && errno == EINTR);
    error = written < 0 ? errno : (written == (ssize_t)length ? 0 : EIO);
    close(fd);
    if (error != 0)
        ALOGW("Cannot write %s: %s", path, strerror(error));
    return -error;
}

static int write_frequency(unsigned int cpu, unsigned int frequency)
{
    char path[128];
    char value[32];

    snprintf(path, sizeof(path),
             "/sys/devices/system/cpu/cpu%u/cpufreq/scaling_max_freq", cpu);
    snprintf(value, sizeof(value), "%u", frequency);
    return write_value(path, value);
}

static unsigned int low_battery_limit(unsigned int cpu)
{
    if (battery_low)
        return cpu == 0 ? 972000U : 594000U;
    return 0;
}

/* Caller holds lock. Only undo a limit that this HAL successfully applied. */
static void update_cpu_limit(unsigned int cpu, int on)
{
    struct cpu_limit *limit = &limits[cpu];
    unsigned int current;
    unsigned int target;
    unsigned int maximum;
    unsigned int battery_limit;
    unsigned int actual;

    current = read_frequency(cpu, "scaling_max_freq");
    if (current == 0)
        return;

    /* A changed cap belongs to an external writer. Treat it as the new
     * baseline, never restore a previous higher value over it.
     */
    if (limit->applied_khz != 0 && current != limit->applied_khz) {
        ALOGI("cpu%u: preserve external cap %u kHz", cpu, current);
        limit->applied_khz = 0;
    }
    if (limit->applied_khz == 0)
        limit->restore_khz = current;
    maximum = read_frequency(cpu, "cpuinfo_max_freq");
    if (maximum == 0)
        return;
    target = limit->restore_khz < maximum ? limit->restore_khz : maximum;
    if (!on && target > SCREEN_OFF_MAX_KHZ)
        target = SCREEN_OFF_MAX_KHZ;
    battery_limit = low_battery_limit(cpu);
    if (battery_limit != 0 && target > battery_limit)
        target = battery_limit;
    if (target != current && write_frequency(cpu, target) != 0)
        return;
    actual = read_frequency(cpu, "scaling_max_freq");
    if (actual == target)
        limit->applied_khz = actual < limit->restore_khz ? actual : 0;
    else {
        /* Failed/changed/rounded readback is not ours to undo. */
        limit->applied_khz = 0;
        ALOGW("cpu%u: requested %u kHz, policy allows %u kHz",
              cpu, target, actual);
    }
}

static void *watch_battery_policy(void *unused)
{
    unsigned int serial = 0;
    char value[PROPERTY_VALUE_MAX];
    int previous = -1;
    unsigned int cpu;
    (void)unused;
    for (;;) {
        /* Futex notification: no polling timer, wake lock, or suspend wakeup.
         * Capture the area serial before reading to avoid a lost update.
         */
        serial = __system_property_wait_any(serial);
        property_get(MIONE_BATTERY_LOW_PROPERTY, value, "0");
        pthread_mutex_lock(&lock);
        battery_low = strcmp(value, "1") == 0;
        if (battery_low != previous) {
            for (cpu = 0; cpu < CPU_COUNT; ++cpu)
                update_cpu_limit(cpu, interactive);
            ALOGI("battery_low=%d; interactive=%d; CPU caps %u/%u kHz",
                  battery_low, interactive,
                  read_frequency(0, "scaling_max_freq"),
                  read_frequency(1, "scaling_max_freq"));
            previous = battery_low;
        }
        pthread_mutex_unlock(&lock);
    }
    return NULL;
}

static void start_battery_watcher(void)
{
    pthread_t thread;
    int error = pthread_create(&thread, NULL, watch_battery_policy, NULL);
    if (error == 0)
        pthread_detach(thread);
    else
        ALOGE("Cannot start battery policy watcher: %s", strerror(error));
}

static void power_init(struct power_module *module)
{
    (void)module;
    pthread_once(&watcher_once, start_battery_watcher);
    ALOGI("MSM8660 Power HAL initialized; screen-off cap %u kHz; "
          "CPU maximums %u/%u kHz", SCREEN_OFF_MAX_KHZ,
          read_frequency(0, "cpuinfo_max_freq"),
          read_frequency(1, "cpuinfo_max_freq"));
}

static void power_set_interactive(struct power_module *module, int on)
{
    unsigned int cpu;

    (void)module;
    pthread_mutex_lock(&lock);
    interactive = !!on;
    for (cpu = 0; cpu < CPU_COUNT; ++cpu)
        update_cpu_limit(cpu, interactive);
    ALOGI("interactive=%d; CPU caps %u/%u kHz", interactive,
          read_frequency(0, "scaling_max_freq"),
          read_frequency(1, "scaling_max_freq"));
    pthread_mutex_unlock(&lock);
}

static void power_hint(struct power_module *module, power_hint_t hint, void *data)
{
    struct timespec now;
    int64_t now_ns;
    unsigned int cpu;

    (void)module;
    /* CM11 passes CPU_BOOST's integer duration as the pointer value. */
    if (hint != POWER_HINT_INTERACTION && hint != POWER_HINT_CPU_BOOST)
        return;
    if (hint == POWER_HINT_CPU_BOOST && (intptr_t)data <= 0)
        return;

    pthread_mutex_lock(&lock);
    if (!interactive || clock_gettime(CLOCK_MONOTONIC, &now) != 0)
        goto out;
    now_ns = (int64_t)now.tv_sec * 1000000000LL + now.tv_nsec;
    if (last_boost_ns != 0 && now_ns - last_boost_ns < BOOST_INTERVAL_NS)
        goto out;
    /* Retry a pending restore if an offline CPU has since come online. */
    for (cpu = 0; cpu < CPU_COUNT; ++cpu)
        update_cpu_limit(cpu, 1);
    /* This kernel provides a one-shot pulse, without boostpulse_duration. */
    if (write_value(BOOSTPULSE_PATH, "1") == 0) {
        last_boost_ns = now_ns;
        ALOGV("interactive boost pulse (hint=%d)", hint);
    }
out:
    pthread_mutex_unlock(&lock);
}

static struct hw_module_methods_t power_methods = {
    .open = NULL,
};

struct power_module HAL_MODULE_INFO_SYM = {
    .common = {
        .tag = HARDWARE_MODULE_TAG,
        .module_api_version = POWER_MODULE_API_VERSION_0_2,
        .hal_api_version = HARDWARE_HAL_API_VERSION,
        .id = POWER_HARDWARE_MODULE_ID,
        .name = "MiOne MSM8660 Power HAL",
        .author = "The CyanogenMod Project",
        .methods = &power_methods,
    },
    .init = power_init,
    .setInteractive = power_set_interactive,
    .powerHint = power_hint,
};

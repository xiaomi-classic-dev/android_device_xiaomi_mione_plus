/* Copyright (C) 2026 The CyanogenMod Project
 * SPDX-License-Identifier: Apache-2.0
 */
#define LOG_TAG "MioneBattery"
#include <cutils/log.h>
#include <cutils/properties.h>
#include <string.h>
#include <healthd.h>
#include "battery_policy.h"

static int low;

void healthd_board_init(struct healthd_config *config)
{
    char value[PROPERTY_VALUE_MAX];
    (void)config;
    property_get(MIONE_BATTERY_LOW_PROPERTY, value, "0");
    low = value[0] == '1';
}

int healthd_board_battery_update(struct android::BatteryProperties *props)
{
    char current[PROPERTY_VALUE_MAX];
    const char *value;
    low = mione_battery_low(low, props->batteryPresent, props->batteryLevel);
    value = low ? "1" : "0";
    property_get(MIONE_BATTERY_LOW_PROPERTY, current, "");
    if (strcmp(current, value) != 0) {
        if (property_set(MIONE_BATTERY_LOW_PROPERTY, value) == 0)
            ALOGI("battery_low=%d; capacity=%d; present=%d", low,
                  props->batteryLevel, props->batteryPresent);
        else
            ALOGE("Cannot publish battery policy");
    }
    return 0;
}

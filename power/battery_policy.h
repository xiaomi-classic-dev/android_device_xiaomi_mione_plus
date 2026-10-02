/* Copyright (C) 2026 The CyanogenMod Project
 * SPDX-License-Identifier: Apache-2.0
 */
#ifndef MIONE_BATTERY_POLICY_H
#define MIONE_BATTERY_POLICY_H

#define MIONE_BATTERY_LOW_PROPERTY "sys.mione.battery_low"

/* Hysteresis also applies while charging: connecting USB does not imply that
 * a depleted battery has recovered. Invalid readings preserve the last state.
 */
static inline int mione_battery_low(int previous, int present, int level)
{
    if (!present || level < 0 || level > 100)
        return previous;
    if (level <= 10)
        return 1;
    if (level >= 15)
        return 0;
    return previous;
}
#endif

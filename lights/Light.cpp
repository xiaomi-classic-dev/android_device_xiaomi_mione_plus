/*
 * Copyright (C) 2008 The Android Open Source Project
 * Copyright (C) 2026 The Xiaomi Classic Device Project
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

#define LOG_TAG "mione_lights"
#include <cutils/log.h>
#include "Light.h"

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LED_PATH(name, attr) "/sys/class/leds/" name "/" attr
#define BACKLIGHT_MAX 127

namespace android { namespace hardware { namespace light { namespace V2_0 { namespace implementation {

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static LightState g_battery;
static LightState g_notification;
static LightState g_rgb;
static int g_rgb_valid;

static const struct {
    const char *brightness;
    const char *blink;
    const char *freq;
    const char *pwm;
} g_leds[] = {
    {LED_PATH("red", "brightness"), LED_PATH("red", "blink"),
     LED_PATH("red", "freq"), LED_PATH("red", "pwm")},
    {LED_PATH("green", "brightness"), LED_PATH("green", "blink"),
     LED_PATH("green", "freq"), LED_PATH("green", "pwm")},
    {LED_PATH("blue", "brightness"), LED_PATH("blue", "blink"),
     LED_PATH("blue", "freq"), LED_PATH("blue", "pwm")},
};

static int write_int(const char *path, int value)
{
    char buffer[20];
    int fd, length, error;
    ssize_t written;

    fd = open(path, O_WRONLY | O_CLOEXEC);
    if (fd < 0) {
        error = -errno;
        ALOGE("open %s: %s", path, strerror(-error));
        return error;
    }
    length = snprintf(buffer, sizeof(buffer), "%d\n", value);
    do {
        written = write(fd, buffer, length);
    } while (written < 0 && errno == EINTR);
    error = written < 0 ? -errno : written == length ? 0 : -EIO;
    close(fd);
    if (error)
        ALOGE("write %s: %s", path, strerror(-error));
    return error;
}

static int is_lit(const LightState *state)
{
    return state->color & 0x00ffffff;
}

static int rgb_to_brightness(const LightState *state)
{
    unsigned int color = state->color;
    return (77 * ((color >> 16) & 0xff) +
            150 * ((color >> 8) & 0xff) + 29 * (color & 0xff)) >> 8;
}

static int set_light_backlight(const LightState *state)
{
    int brightness = rgb_to_brightness(state);
    int level = (brightness * BACKLIGHT_MAX + 127) / 255;
    int error;

    /* LM3530 exposes 7-bit brightness; keep a nonzero request lit. */
    if (brightness && !level)
        level = 1;
    pthread_mutex_lock(&g_lock);
    error = write_int(LED_PATH("backlight", "brightness"), level);
    pthread_mutex_unlock(&g_lock);
    return error;
}

static int set_light_buttons(const LightState *state)
{
    int error;
    pthread_mutex_lock(&g_lock);
    error = write_int(LED_PATH("button-backlight", "brightness"),
                      is_lit(state) ? 255 : 0);
    pthread_mutex_unlock(&g_lock);
    return error;
}

static int stop_blink_locked(void)
{
    int i, error = 0;
    for (i = 0; i < 3; ++i) {
        int result = write_int(g_leds[i].blink, 0);
        if (!error)
            error = result;
    }
    return error;
}

static int set_rgb_locked(const LightState *state)
{
    int i, error, channels[3], freq = 0, pwm = 0;
    unsigned int alpha = state->color >> 24;
    int timed = state->flashMode == Flash::TIMED &&
                state->flashOnMs > 0 && state->flashOffMs > 0;

    if (g_rgb_valid && state->color == g_rgb.color &&
        state->flashMode == g_rgb.flashMode &&
        state->flashOnMs == g_rgb.flashOnMs &&
        state->flashOffMs == g_rgb.flashOffMs)
        return 0;
    g_rgb_valid = 0;

    /* CM12.1 carries the LED brightness setting in alpha. Zero also occurs
     * in legacy RGB callers, where it means full brightness. */
    if (!alpha)
        alpha = 255;
    for (i = 0; i < 3; ++i) {
        unsigned int channel = (state->color >> (16 - i * 8)) & 0xff;
        channels[i] = channel * alpha / 255;
        /* PM8058 ignores the low four brightness bits. */
        if (channel && channels[i] < 16)
            channels[i] = 16;
    }

    if (timed) {
        int64_t total = (int64_t)state->flashOnMs + state->flashOffMs;
        int64_t period = total / 50;
        /* The kernel computes pwm * freq * 50 in a 32-bit value. */
        if (period > INT_MAX / (255 * 50))
            period = INT_MAX / (255 * 50);
        freq = period ? (int)period : 1;
        pwm = (int)((int64_t)state->flashOnMs * 255 / total);
        if (pwm < 1)
            pwm = 1;
        if (pwm > 254)
            pwm = 254;
    }

    /* MiOne only applies blink changes on a zero/nonzero transition. Stop
     * all channels before changing their shared LUT timing or amplitude. */
    error = stop_blink_locked();
    if (error)
        return error;
    for (i = 0; i < 3; ++i) {
        error = write_int(g_leds[i].brightness, channels[i]);
        if (error)
            return error;
        if (timed) {
            error = write_int(g_leds[i].freq, freq);
            if (!error)
                error = write_int(g_leds[i].pwm, pwm);
            if (error)
                return error;
        }
    }
    if (timed) {
        for (i = 0; i < 3; ++i) {
            /* This driver's blink value is the LED current, not a boolean. */
            error = write_int(g_leds[i].blink, channels[i]);
            if (error) {
                stop_blink_locked();
                return error;
            }
        }
    }
    g_rgb = *state;
    g_rgb_valid = 1;
    return 0;
}

static int set_shared_light(LightState *saved,
                            const LightState *state)
{
    int error;
    pthread_mutex_lock(&g_lock);
    *saved = *state;
    /* Retain the shipped HAL's battery-over-notification priority. */
    error = set_rgb_locked(is_lit(&g_battery) ? &g_battery : &g_notification);
    pthread_mutex_unlock(&g_lock);
    return error;
}

Return<Status> Light::setLight(Type type, const LightState& state) {
    int error;
    switch (type) {
    case Type::BACKLIGHT:
        if (state.brightnessMode == Brightness::LOW_PERSISTENCE)
            return Status::BRIGHTNESS_NOT_SUPPORTED;
        error = set_light_backlight(&state);
        break;
    case Type::BUTTONS:
        error = set_light_buttons(&state);
        break;
    case Type::BATTERY:
        error = set_shared_light(&g_battery, &state);
        break;
    case Type::NOTIFICATIONS:
        error = set_shared_light(&g_notification, &state);
        break;
    default:
        return Status::LIGHT_NOT_SUPPORTED;
    }
    return error == 0 ? Status::SUCCESS : Status::UNKNOWN;
}

Return<void> Light::getSupportedTypes(getSupportedTypes_cb cb) {
    hidl_vec<Type> types;
    types.resize(4);
    types[0] = Type::BACKLIGHT;
    types[1] = Type::BUTTONS;
    types[2] = Type::BATTERY;
    types[3] = Type::NOTIFICATIONS;
    cb(types);
    return Void();
}

}}}}}  // android::hardware::light::V2_0::implementation

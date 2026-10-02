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
#include <hardware/lights.h>

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

static pthread_mutex_t g_lock = PTHREAD_MUTEX_INITIALIZER;
static struct light_state_t g_battery;
static struct light_state_t g_notification;
static struct light_state_t g_rgb;
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

static int is_lit(const struct light_state_t *state)
{
    return state->color & 0x00ffffff;
}

static int rgb_to_brightness(const struct light_state_t *state)
{
    unsigned int color = state->color;
    return (77 * ((color >> 16) & 0xff) +
            150 * ((color >> 8) & 0xff) + 29 * (color & 0xff)) >> 8;
}

static int set_light_backlight(struct light_device_t *dev,
                               const struct light_state_t *state)
{
    int brightness = rgb_to_brightness(state);
    int level = (brightness * BACKLIGHT_MAX + 127) / 255;
    int error;
    (void)dev;

    /* LM3530 exposes 7-bit brightness; keep a nonzero request lit. */
    if (brightness && !level)
        level = 1;
    pthread_mutex_lock(&g_lock);
    error = write_int(LED_PATH("backlight", "brightness"), level);
    pthread_mutex_unlock(&g_lock);
    return error;
}

static int set_light_buttons(struct light_device_t *dev,
                             const struct light_state_t *state)
{
    int error;
    (void)dev;
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

static int set_rgb_locked(const struct light_state_t *state)
{
    int i, error, channels[3], freq = 0, pwm = 0;
    unsigned int alpha = state->color >> 24;
    int timed = state->flashMode == LIGHT_FLASH_TIMED &&
                state->flashOnMS > 0 && state->flashOffMS > 0;

    if (g_rgb_valid && state->color == g_rgb.color &&
        state->flashMode == g_rgb.flashMode &&
        state->flashOnMS == g_rgb.flashOnMS &&
        state->flashOffMS == g_rgb.flashOffMS)
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
        int64_t total = (int64_t)state->flashOnMS + state->flashOffMS;
        int64_t period = total / 50;
        /* The kernel computes pwm * freq * 50 in a 32-bit value. */
        if (period > INT_MAX / (255 * 50))
            period = INT_MAX / (255 * 50);
        freq = period ? (int)period : 1;
        pwm = (int)((int64_t)state->flashOnMS * 255 / total);
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

static int set_shared_light(struct light_state_t *saved,
                            const struct light_state_t *state)
{
    int error;
    pthread_mutex_lock(&g_lock);
    *saved = *state;
    /* Retain the shipped HAL's battery-over-notification priority. */
    error = set_rgb_locked(is_lit(&g_battery) ? &g_battery : &g_notification);
    pthread_mutex_unlock(&g_lock);
    return error;
}

static int set_light_battery(struct light_device_t *dev,
                             const struct light_state_t *state)
{
    (void)dev;
    return set_shared_light(&g_battery, state);
}

static int set_light_notifications(struct light_device_t *dev,
                                   const struct light_state_t *state)
{
    (void)dev;
    return set_shared_light(&g_notification, state);
}

static int close_lights(struct hw_device_t *dev)
{
    free(dev);
    return 0;
}

static int open_lights(const struct hw_module_t *module, const char *name,
                       struct hw_device_t **device)
{
    struct light_device_t *dev;
    int (*set_light)(struct light_device_t *, const struct light_state_t *);

    if (!name || !device)
        return -EINVAL;
    *device = NULL;
    if (!strcmp(name, LIGHT_ID_BACKLIGHT))
        set_light = set_light_backlight;
    else if (!strcmp(name, LIGHT_ID_BUTTONS))
        set_light = set_light_buttons;
    else if (!strcmp(name, LIGHT_ID_BATTERY))
        set_light = set_light_battery;
    else if (!strcmp(name, LIGHT_ID_NOTIFICATIONS))
        set_light = set_light_notifications;
    else
        return -EINVAL;

    dev = calloc(1, sizeof(*dev));
    if (!dev)
        return -ENOMEM;
    dev->common.tag = HARDWARE_DEVICE_TAG;
    dev->common.version = 0;
    dev->common.module = (struct hw_module_t *)module;
    dev->common.close = close_lights;
    dev->set_light = set_light;
    *device = &dev->common;
    return 0;
}

static struct hw_module_methods_t lights_module_methods = {
    .open = open_lights,
};

struct hw_module_t HAL_MODULE_INFO_SYM = {
    .tag = HARDWARE_MODULE_TAG,
    .module_api_version = 1,
    .hal_api_version = HARDWARE_HAL_API_VERSION,
    .id = LIGHTS_HARDWARE_MODULE_ID,
    .name = "MiOne LM3530/PM8058 lights",
    .author = "The Android Open Source Project, Xiaomi Classic Device Project",
    .methods = &lights_module_methods,
};

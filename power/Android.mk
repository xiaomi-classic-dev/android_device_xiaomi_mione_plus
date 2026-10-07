# Copyright (C) 2026 The CyanogenMod Project
# SPDX-License-Identifier: Apache-2.0

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := power.msm8660
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := power.c
LOCAL_SHARED_LIBRARIES := liblog libcutils
LOCAL_CFLAGS := -Wall -Wextra -Werror
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libhealthd.mione
LOCAL_SRC_FILES := healthd_mione.cpp
LOCAL_HEADER_LIBRARIES := libhealthd_headers
LOCAL_CFLAGS := -Wall -Wextra -Werror
include $(BUILD_STATIC_LIBRARY)

include $(LOCAL_PATH)/tests/Android.mk

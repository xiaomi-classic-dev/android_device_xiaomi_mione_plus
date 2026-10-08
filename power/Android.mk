# Copyright (C) 2026 The CyanogenMod Project
# SPDX-License-Identifier: Apache-2.0

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.power@1.0-service.mione
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := android.hardware.power@1.0-service.mione.rc
LOCAL_SRC_FILES := power.c Power.cpp service.cpp
LOCAL_SHARED_LIBRARIES := liblog libcutils libutils libhidlbase libhidltransport android.hardware.power@1.0
LOCAL_CFLAGS := -Wall -Wextra -Werror
include $(BUILD_EXECUTABLE)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.health@2.0-service.mione
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_OVERRIDES_MODULES := healthd android.hardware.health@2.0-service
LOCAL_INIT_RC := android.hardware.health@2.0-service.mione.rc
LOCAL_SRC_FILES := health_service.cpp
LOCAL_STATIC_LIBRARIES := libhealthd.mione libhealthservice \
    android.hardware.health@2.0-impl android.hardware.health@1.0-convert \
    libhealthstoragedefault libbatterymonitor
LOCAL_SHARED_LIBRARIES := libbase libcutils libhidlbase libhidltransport \
    libhwbinder liblog libutils android.hardware.health@2.0
LOCAL_CFLAGS := -Wall -Wextra -Werror
include $(BUILD_EXECUTABLE)

include $(LOCAL_PATH)/tests/Android.mk

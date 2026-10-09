LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := mione_power_probe
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := tests
LOCAL_SRC_FILES := power_probe.cpp
LOCAL_HEADER_LIBRARIES := libhealthd_headers
LOCAL_CFLAGS := -Wall -Wextra -Werror
LOCAL_STATIC_LIBRARIES := libhealthd.mione
LOCAL_SHARED_LIBRARIES := libcutils libutils liblog libhidlbase android.hardware.power@1.0
include $(BUILD_EXECUTABLE)

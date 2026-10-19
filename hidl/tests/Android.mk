LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := mione_hidl_probe
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := tests
LOCAL_SRC_FILES := probe.cpp
LOCAL_CFLAGS := -Wall -Wextra -Werror
LOCAL_SHARED_LIBRARIES := libutils libhidlbase libbinder_ndk \
    android.hardware.light-V2-ndk android.hardware.power-V3-ndk \
    android.hardware.thermal@1.0
include $(BUILD_EXECUTABLE)

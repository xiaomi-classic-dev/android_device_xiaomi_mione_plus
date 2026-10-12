LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := audio.bluetooth.msm8660
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_VENDOR_MODULE := true
LOCAL_MULTILIB := 32
LOCAL_SRC_FILES := bluetooth_audio.cpp
LOCAL_CPP_STD := c++17
LOCAL_CFLAGS := -Wall -Wextra -Werror
LOCAL_SHARED_LIBRARIES := libdl liblog libhidlbase libutils android.hardware.bluetooth.audio@2.0
include $(BUILD_SHARED_LIBRARY)

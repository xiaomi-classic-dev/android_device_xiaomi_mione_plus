LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := sensors.msm8660
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_SRC_FILES := sensors_compat.cpp
LOCAL_SHARED_LIBRARIES := libdl liblog
LOCAL_CFLAGS := -Wall -Wextra -Werror
include $(BUILD_SHARED_LIBRARY)

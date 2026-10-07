LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.light@2.0-service.mione
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := android.hardware.light@2.0-service.mione.rc
LOCAL_SRC_FILES := Light.cpp service.cpp
LOCAL_SHARED_LIBRARIES := liblog libutils libhidlbase libhidltransport android.hardware.light@2.0
LOCAL_CFLAGS := -Wall -Wextra -Werror
include $(BUILD_EXECUTABLE)

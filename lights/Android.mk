LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.light-service.mione
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := android.hardware.light-service.mione.rc
LOCAL_SRC_FILES := Light.cpp service.cpp
LOCAL_VINTF_FRAGMENTS := light-mione.xml
LOCAL_SHARED_LIBRARIES := liblog libbinder_ndk android.hardware.light-V2-ndk
LOCAL_CFLAGS := -Wall -Wextra -Werror
include $(BUILD_EXECUTABLE)

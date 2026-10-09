LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.thermal@1.0-service.mione
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE_TAGS := optional
LOCAL_INIT_RC := android.hardware.thermal@1.0-service.mione.rc
LOCAL_SRC_FILES := Thermal.cpp service.cpp
LOCAL_SHARED_LIBRARIES := libbase liblog libutils libhidlbase android.hardware.thermal@1.0
LOCAL_CFLAGS := -Wall -Wextra -Werror
include $(BUILD_EXECUTABLE)

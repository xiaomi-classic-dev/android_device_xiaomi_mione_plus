LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libril_mione
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := ril_mione.c
LOCAL_C_INCLUDES := hardware/ril/include
LOCAL_CFLAGS := -DRIL_SHLIB -Wall -Wextra -Werror
LOCAL_SHARED_LIBRARIES := libdl liblog
include $(BUILD_SHARED_LIBRARY)

include $(LOCAL_PATH)/tests/Android.mk

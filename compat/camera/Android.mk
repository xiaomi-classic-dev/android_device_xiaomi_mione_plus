LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libmione_camera_shim
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := poll_compat.c
LOCAL_SHARED_LIBRARIES := libdl
LOCAL_CFLAGS := -Wall -Wextra -Werror
include $(BUILD_SHARED_LIBRARY)

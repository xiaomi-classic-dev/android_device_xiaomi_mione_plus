LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)
LOCAL_MODULE := libEGL_mione_compat
LOCAL_VENDOR_MODULE := true
LOCAL_MULTILIB := 32
LOCAL_MIN_SDK_VERSION := 26
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := legacy_dequeue.c
LOCAL_CFLAGS := -Wall -Wextra -Werror -fvisibility=hidden
LOCAL_HEADER_LIBRARIES := libnativewindow_headers libarect_headers libnativebase_headers libcutils_headers libsystem_headers
LOCAL_SHARED_LIBRARIES := liblog libnativewindow
include $(BUILD_SHARED_LIBRARY)

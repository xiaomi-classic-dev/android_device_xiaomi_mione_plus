LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES:= readmac.c
LOCAL_MODULE:= libreadmac
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SHARED_LIBRARIES := libnv liboncrpc liblog

include $(BUILD_STATIC_LIBRARY)


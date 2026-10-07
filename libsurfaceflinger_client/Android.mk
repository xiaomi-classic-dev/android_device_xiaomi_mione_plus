ifeq ($(CAMERA_USES_SURFACEFLINGER_CLIENT_STUB),true)

LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_SRC_FILES :=

LOCAL_SHARED_LIBRARIES :=

LOCAL_MODULE := libsurfaceflinger_client
LOCAL_VENDOR_MODULE := true

include $(BUILD_SHARED_LIBRARY)

endif

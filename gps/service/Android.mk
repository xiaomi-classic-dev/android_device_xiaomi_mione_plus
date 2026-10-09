LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.gnss@1.0-service.mione
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
# Reuse the platform service; only its device access groups are MiOne-specific.
LOCAL_SRC_FILES := ../../../../../hardware/interfaces/gnss/1.0/default/service.cpp
LOCAL_INIT_RC := android.hardware.gnss@1.0-service.mione.rc
LOCAL_SHARED_LIBRARIES := liblog libcutils libdl libbase libutils libhardware \
    libbinder libhidlbase android.hardware.gnss@1.0
include $(BUILD_EXECUTABLE)

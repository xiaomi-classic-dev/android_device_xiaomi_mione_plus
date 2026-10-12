LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.gnss@1.0-service.mione
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_PROPRIETARY_MODULE := true
# The legacy modem RPC receiver acquires system Suspend wakelocks via libpower.
LOCAL_SRC_FILES := service.cpp
LOCAL_INIT_RC := android.hardware.gnss@1.0-service.mione.rc
LOCAL_SHARED_LIBRARIES := liblog libcutils libdl libbase libutils libhardware \
    libbinder libhidlbase android.hardware.gnss@1.0
include $(BUILD_EXECUTABLE)

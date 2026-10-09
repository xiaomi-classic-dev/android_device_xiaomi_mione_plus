ifneq ($(TARGET_PROVIDES_GPS_LOC_API),true)
ifneq ($(BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE),)
ifneq ($(BUILD_TINY_ANDROID),true)
#Compile this library only for builds with the latest modem image

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

## Libs
LOCAL_SHARED_LIBRARIES := \
    libutils \
    libcutils

LOCAL_SRC_FILES += \
    loc_log.cpp \
    loc_cfg.cpp \
    msg_q.c \
    linked_list.c

LOCAL_CFLAGS += \
     -fno-short-enums \
     -D_ANDROID_

## Includes
LOCAL_C_INCLUDES:=



LOCAL_MODULE := libgps.utils
LOCAL_VENDOR_MODULE := true

LOCAL_MODULE_TAGS := optional

LOCAL_PRELINK_MODULE := false

include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mione_gps_utils_headers
LOCAL_VENDOR_MODULE := true
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
include $(BUILD_HEADER_LIBRARY)
endif # not BUILD_TINY_ANDROID
endif # BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE
endif#TARGET_PROVIDES_GPS_LOC_API

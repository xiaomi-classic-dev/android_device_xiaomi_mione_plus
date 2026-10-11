HAL3ON1_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_PATH := $(HAL3ON1_PATH)
LOCAL_C_INCLUDES := \
    system/media/camera/include \
    frameworks/native/include \
    external/libyuv/files/include \
    $(call project-path-for,qcom-display)/libgralloc

LOCAL_SRC_FILES := \
    HAL3on1-adapter.cpp

LOCAL_SHARED_LIBRARIES := \
    libhardware \
    liblog \
    libutils \
    libcutils \
    libbase \
    libcamera_metadata \
    libui \
    android.hidl.token@1.0-utils \
    android.hardware.graphics.bufferqueue@1.0 \
    libbinder \
    libjpeg

LOCAL_STATIC_LIBRARIES := \
    android.hardware.camera.common@1.0-helper \
    libarect \
    libyuv_static

LOCAL_CPPFLAGS += -DLOG_NDEBUG

ifeq ($(TARGET_SYSFS_FLASH_PATH_BRIGHTNESS),)
    TARGET_SYSFS_FLASH_PATH_BRIGHTNESS := /sys/class/leds/flashlight/brightness
endif

ifeq ($(TARGET_SYSFS_FLASH_PATH_BRIGHTNESS_FALLBACK),)
    TARGET_SYSFS_FLASH_PATH_BRIGHTNESS_FALLBACK := /sys/class/leds/flashlight/brightness
endif

ifneq ($(TARGET_SYSFS_FLASH_PATH_BRIGHTNESS),)
    LOCAL_CFLAGS += -DSYSFS_FLASH_PATH_BRIGHTNESS=\"$(TARGET_SYSFS_FLASH_PATH_BRIGHTNESS)\"
endif

ifneq ($(TARGET_SYSFS_FLASH_PATH_BRIGHTNESS_FALLBACK),)
    LOCAL_CFLAGS += -DSYSFS_FLASH_PATH_BRIGHTNESS_FALLBACK=\"$(TARGET_SYSFS_FLASH_PATH_BRIGHTNESS_FALLBACK)\"
endif

LOCAL_HEADER_LIBRARIES := libnativebase_headers
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_MODULE := camera.$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS := optional
LOCAL_32_BIT_ONLY := true
LOCAL_PROPRIETARY_MODULE := true

include $(BUILD_SHARED_LIBRARY)

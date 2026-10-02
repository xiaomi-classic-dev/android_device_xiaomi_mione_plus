# MiOne MSM8660 legacy CAF HAL1.
ifeq ($(BOARD_MIONE_SOURCE_CAMERA),true)
LOCAL_PATH := $(call my-dir)
MIONE_DEVICE_CAMERA_PATH := $(LOCAL_PATH)
include $(CLEAR_VARS)
LOCAL_MODULE := camera.msm8660
LOCAL_MODULE_PATH := $(TARGET_OUT_SHARED_LIBRARIES)/hw
LOCAL_MODULE_TAGS := optional
LOCAL_LDFLAGS := -Wl,--version-script=$(LOCAL_PATH)/hal.exports
LOCAL_ADDITIONAL_DEPENDENCIES := $(LOCAL_PATH)/hal.exports
LOCAL_ADDITIONAL_DEPENDENCIES += $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr
LOCAL_SRC_FILES := QCameraHAL.cpp QCameraHWI_Parm.cpp QCameraHWI.cpp \
    QCameraHWI_Preview.cpp QCameraHWI_Record.cpp QCameraHWI_Still.cpp \
    QCameraHWI_Mem.cpp QCameraHWI_Display.cpp QCameraStream.cpp \
    QualcommCamera2.cpp QCameraHWI_Rdi.cpp QCameraParameters.cpp
LOCAL_CFLAGS := -DUSE_ION -DHW_ENCODE -D_ANDROID_ \
    -DNUM_PREVIEW_BUFFERS=4 -DUSE_NEON_CONVERSION \
    -DMSM_CAMERA_BIONIC -DMSM_CAMERA_GCC \
    -DCAMERA_ION_HEAP_ID=ION_CP_MM_HEAP_ID \
    -DCAMERA_ZSL_ION_HEAP_ID=ION_CP_MM_HEAP_ID \
    -DCAMERA_ION_FALLBACK_HEAP_ID=ION_CAMERA_HEAP_ID \
    -DCAMERA_ZSL_ION_FALLBACK_HEAP_ID=ION_CAMERA_HEAP_ID \
    -DCAMERA_GRALLOC_HEAP_ID=GRALLOC_USAGE_PRIVATE_CAMERA_HEAP \
    -DCAMERA_GRALLOC_FALLBACK_HEAP_ID=GRALLOC_USAGE_PRIVATE_CAMERA_HEAP \
    -DCAMERA_GRALLOC_CACHING_ID=0 \
    -include bionic/libc/include/sys/socket.h
LOCAL_C_INCLUDES := $(LOCAL_PATH) $(LOCAL_PATH)/mm-camera-interface \
    $(LOCAL_PATH)/inc $(TARGET_OUT_INTERMEDIATES)/KERNEL_OBJ/usr/include \
    system/media/camera/include \
    $(call project-path-for,qcom-display)/libgralloc \
    $(call project-path-for,qcom-display)/libgenlock \
    $(call project-path-for,qcom-media)/mm-core/inc \
    $(call project-path-for,qcom-media)/libstagefrighthw
LOCAL_SHARED_LIBRARIES := libutils libui libcamera_client liblog libcutils \
    libmmcamera_interface2 libgenlock libbinder libdl
include $(BUILD_SHARED_LIBRARY)
include $(MIONE_DEVICE_CAMERA_PATH)/mm-camera-interface/Android.mk
endif

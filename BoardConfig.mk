# Copyright (C) 2009 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

#
# This file sets variables that control the way modules are built
# thorughout the system. It should not be used to conditionally
# disable makefiles (the proper mechanism to control what gets
# included in a build is to use PRODUCT_PACKAGES in a product
# definition file).
#

# WARNING: This line must come *before* including the proprietary
# variant, so that it gets overwritten by the parent (which goes
# against the traditional rules of inheritance).

MIONE_PATH := device/xiaomi/mione_plus
BOARD_VENDOR := xiaomi

# Memfd
TARGET_HAS_MEMFD_BACKPORT := true

# Bootloader
TARGET_NO_BOOTLOADER := true

# Platform
TARGET_BOARD_PLATFORM := msm8660
TARGET_BOARD_PLATFORM_GPU := qcom-adreno200

# Architecture
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
# Oreo Soong has no Scorpion variant; keep ARMv7/NEON with generic tuning.
TARGET_CPU_VARIANT := generic
TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_SMP := true
ARCH_ARM_HAVE_TLS_REGISTER := true
TARGET_NEEDS_PLATFORM_TEXT_RELOCATIONS := true

# QCOM hardware
BOARD_USES_QCOM_HARDWARE := true

# Keep the MSM8660 HAL repositories at their existing standalone paths.
USE_DEVICE_SPECIFIC_AUDIO := true
DEVICE_SPECIFIC_AUDIO_PATH := hardware/qcom/audio-caf/msm8660
USE_DEVICE_SPECIFIC_DISPLAY := true
DEVICE_SPECIFIC_DISPLAY_PATH := hardware/qcom/display-caf/msm8660
USE_DEVICE_SPECIFIC_MEDIA := true
DEVICE_SPECIFIC_MEDIA_PATH := hardware/qcom/media-caf/msm8660
# These legacy HALs use Android.mk and need no consolidated QCOM namespace.
QCOM_SOONG_NAMESPACE :=
BOARD_GLOBAL_CFLAGS += -DQCOM_HARDWARE
TARGET_ENABLE_QC_AV_ENHANCEMENTS := true
TARGET_USE_QCOM_BIONIC_OPTIMIZATION := true

# QCOM BSP
TARGET_USES_QCOM_BSP := true
BOARD_GLOBAL_CFLAGS += -DQCOM_BSP

# Recovery
USE_SET_METADATA := false

# Graphics
BOARD_EGL_NEEDS_LEGACY_FB := true
BOARD_USE_MHEAP_SCREENSHOT := true
USE_OPENGL_RENDERER := true
TARGET_USES_ION := true
TARGET_USES_C2D_COMPOSITION := true
NUM_FRAMEBUFFER_SURFACE_BUFFERS := 3
TARGET_DISPLAY_INSECURE_MM_HEAP := true
TARGET_NO_HW_VSYNC := true
TARGET_RUNNING_WITHOUT_SYNC_FRAMEWORK := true
# Pie validates usage before forwarding it to the legacy gralloc HAL.
# MSM8660 camera heap (bit 22) and uncached memory (bit 25).
TARGET_ADDITIONAL_GRALLOC_10_USAGE_BITS := 0x02402000U
BOARD_EGL_CFG := device/xiaomi/mione_plus/configs/egl.cfg
BOARD_GLOBAL_CFLAGS += -DQCOM_NO_SECURE_PLAYBACK
BOARD_GLOBAL_CFLAGS += -DREFRESH_RATE=60

# Audio
BOARD_USES_LEGACY_ALSA_AUDIO := true
# The legacy CAF policy uses M-only internals; use the Oreo framework manager.
override USE_CUSTOM_AUDIO_POLICY := 0
USE_XML_AUDIO_POLICY_CONF := 1
BOARD_QCOM_VOIP_ENABLED := true
BOARD_QCOM_TUNNEL_LPA_ENABLED := false
BOARD_GLOBAL_CFLAGS += -DLEGACY_QCOM_VOICE

# Bluetooth
BOARD_HAVE_BLUETOOTH := true

# Camera
TARGET_USES_MEDIA_EXTENSIONS := true
BOARD_USES_QCOM_LEGACY_CAM_PARAMS := true
BOARD_NEEDS_MEMORYHEAPPMEM := true
#TARGET_DISABLE_ARM_PIE := true
BOARD_GLOBAL_CFLAGS += -DICS_CAMERA_BLOB
BOARD_GLOBAL_CFLAGS += -DNO_UPDATE_PREVIEW
BOARD_GLOBAL_CFLAGS += -DNEEDS_VECTORIMPL_SYMBOLS

# Legacy camera dependencies
CAMERA_USES_SURFACEFLINGER_CLIENT_STUB := true
BOARD_GLOBAL_CFLAGS += -DDISABLE_HW_ID_MATCH_CHECK

# Limit fixed-address execution to the shipped CAF executables.
LINKER_NON_PIE_EXECUTABLES_HEADER_DIR := $(MIONE_PATH)/include

# Oreo linker shims are selected at build time.
TARGET_LD_SHIM_LIBS := \
    /vendor/bin/cnd|libmione_cnd_shim.so \
    /vendor/bin/netmgrd|libmione_cnd_shim.so \
    /vendor/lib/libnetmgr.so|libmione_cnd_shim.so \
    /vendor/lib/hw/sensors.vendor.msm8660.so|libmione_sensors_shim.so \
    /vendor/bin/mm-qcamera-daemon|libmione_camera_unwind_shim.so \
    /vendor/bin/hw/android.hardware.camera.provider@2.4-service|libmione_camera_unwind_shim.so

# Misc
BOARD_USES_LEGACY_MMAP := true

# SELinux
-include device/qcom/sepolicy-legacy/sepolicy.mk

# Legacy vendor baseline: stock MIUI 4.12.5 (Android 4.1.2).
# Stock has no SPL metadata; use its 2014-12-05 build date as the reference.
VENDOR_SECURITY_PATCH := 2014-12-05

# Filesystem
#BOARD_VOLD_MAX_PARTITIONS := 36

# FM Radio
#BOARD_HAVE_QCOM_FM := true
#BOARD_GLOBAL_CFLAGS += -DQCOM_FM_ENABLED

# GPS
USE_DEVICE_SPECIFIC_GPS := true
DEVICE_SPECIFIC_GPS_PATH := $(MIONE_PATH)/gps
USE_DEVICE_SPECIFIC_LOC_API := true
DEVICE_SPECIFIC_LOC_API_PATH := $(MIONE_PATH)/gps/loc_api
BOARD_USES_QCOM_GPS := true
TARGET_GPS_HAL_PATH := $(MIONE_PATH)/gps
BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := msm8660
BOARD_VENDOR_QCOM_GPS_LOC_API_AMSS_VERSION := 50000

# Webkit
ENABLE_WEBGL := true
TARGET_FORCE_CPU_UPLOAD := true

BOARD_HAVE_XIAOMI_MIONE := true

# Bootloader
TARGET_BOOTLOADER_BOARD_NAME := mione

# Kernel
BOARD_KERNEL_BASE := 0x40200000
BOARD_KERNEL_CMDLINE := console=ttyHSL0,115200,n8 androidboot.hardware=qcom kgsl.mmutype=gpummu vmalloc=400M androidboot.selinux=permissive
BOARD_KERNEL_PAGE_SIZE := 2048
BOARD_MKBOOTIMG_ARGS := --ramdisk_offset 0x02000000
TARGET_KERNEL_SOURCE := kernel/xiaomi/mione_plus
TARGET_KERNEL_ARCH := arm
TARGET_KERNEL_CONFIG := mione-user_defconfig
TARGET_KERNEL_CROSS_COMPILE_PREFIX := arm-linux-androideabi-
KERNEL_TOOLCHAIN := $(shell pwd)/prebuilts/gcc/linux-x86/arm/arm-linux-androideabi-4.9/bin
# Install modules with the device vendor HALs and matching Wi-Fi loader.
NEED_KERNEL_MODULE_SYSTEM := false

# Bluetooth
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR ?= device/xiaomi/mione_plus/bluetooth
BOARD_HAVE_BLUETOOTH := true
BOARD_HAVE_BLUETOOTH_BCM := true
BOARD_CUSTOM_BT_CONFIG := device/xiaomi/mione_plus/bluetooth/vnd_mione_plus.txt

BOARD_SEPOLICY_DIRS += device/xiaomi/mione_plus/sepolicy

# Display
TARGET_SCREEN_WIDTH := 480
TARGET_SCREEN_HEIGHT := 854

# MiOne's LM3530/PM8058 sysfs contract differs from the generic CAF lights HAL.
TARGET_PROVIDES_LIBLIGHT := true

# QCOM GPS
BOARD_VENDOR_QCOM_GPS_LOC_API_HARDWARE := msm8660

# NFC
BOARD_HAVE_NFC := false

# Radio
TARGET_RIL_VARIANT := caf

# Filesystem
TARGET_FS_CONFIG_GEN := $(MIONE_PATH)/config.fs
TARGET_USERIMAGES_USE_EXT4 := true
BOARD_BOOTIMAGE_PARTITION_SIZE := 10485760
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 20971520
# The current MiOne partition layout provides 1 GiB at mmcblk0p15.
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 1073741824
# Raw capacities from the device partition table (cache p19, userdata p20).
BOARD_CACHEIMAGE_PARTITION_SIZE := 188743680
BOARD_CACHEIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_USERDATAIMAGE_PARTITION_SIZE := 2537488384
BOARD_FLASH_BLOCK_SIZE := 32768
BOARD_VOLD_MAX_PARTITIONS := 28
BOARD_VOLD_EMMC_SHARES_DEV_MAJOR := true

# Recovery
BOARD_RECOVERY_SWIPE := true
BOARD_USES_MMCUTILS := true
BOARD_HAS_NO_SELECT_BUTTON := true
BOARD_UMS_LUNFILE := "/sys/class/android_usb/android0/f_mass_storage/lun/file"
TARGET_USE_CUSTOM_LUN_FILE_PATH := "/sys/class/android_usb/android0/f_mass_storage/lun/file"
TARGET_RECOVERY_FSTAB := device/xiaomi/mione_plus/rootdir/etc/fstab.qcom
RECOVERY_FSTAB_VERSION := 2

# Legacy Qualcomm HAL1, built against the MiOne camera ABI.
USE_DEVICE_SPECIFIC_CAMERA := true
TARGET_NEEDS_LEGACY_CAMERA_HAL1_DYN_NATIVE_HANDLE := true

# Publish device battery policy through healthd's standard board hook.
BOARD_HAL_STATIC_LIBRARIES += libhealthd.mione

# Oreo keeps the legacy 32-bit Binder ABI used by MiOne's userspace.
TARGET_USES_64_BIT_BINDER := false
BOARD_KERNEL_IMAGE_NAME := zImage
DEVICE_MANIFEST_FILE := $(MIONE_PATH)/manifest.xml

# Preserve the API-22 loading contract only in services using legacy blobs.
# /vendor resolves to /system/vendor on this non-Treble device.
TARGET_PROCESS_SDK_VERSION_OVERRIDE := \
    /system/bin/cameraserver=22 \
    /system/bin/mediaserver=22 \
    /system/bin/audioserver=22 \
    /system/vendor/bin/hw/android.hardware.camera.provider@2.4-service=22 \
    /system/vendor/bin/hw/android.hardware.audio@2.0-service=22 \
    /system/vendor/bin/hw/android.hardware.sensors@1.0-service=22 \
    /system/vendor/bin/hw/android.hardware.gnss@1.0-service.mione=22 \
    /system/vendor/bin/hw/android.hardware.media.omx@1.0-service=22 \
    /system/vendor/bin/hw/rild=22 \
    /system/vendor/bin/mione_bdaddr=22 \
    /system/vendor/bin/cnd=22 \
    /system/vendor/bin/mm-pp-daemon=22 \
    /system/vendor/bin/mm-qcamera-daemon=22 \
    /system/vendor/bin/netmgrd=22 \
    /system/vendor/bin/qmuxd=22 \
    /system/vendor/bin/rmt_storage=22 \
    /system/vendor/bin/thermald=22

# Fit the image and dex metadata in the verified 1 GiB system partition.
MALLOC_SVELTE := true
EXCLUDE_SERIF_FONTS := true
USE_REDUCED_CJK_FONT_WEIGHTS := true
WITH_DEXPREOPT_BOOT_IMG_AND_SYSTEM_SERVER_ONLY := true
TARGET_USES_MKE2FS := true

# Non-Treble ramdisk mount points, available before vendor init is parsed.
BOARD_ROOT_EXTRA_FOLDERS += firmware persist

# BCM4329 firmware cannot reconfigure its SoftAP MAC at runtime.
WIFI_HIDL_FEATURE_DISABLE_AP_MAC_RANDOMIZATION := true

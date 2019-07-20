USE_CAMERA_STUB := true

LOCAL_PATH := device/xiaomi/mione_plus

# inherit from the proprietary version
-include vendor/xiaomi/mione_plus/BoardConfigVendor.mk

TARGET_ARCH := arm
TARGET_NO_BOOTLOADER := true
TARGET_BOARD_PLATFORM := msm8660
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_ARCH_VARIANT := armv7-a-neon
TARGET_CPU_VARIANT := generic
TARGET_HARDWARE := qcom
ARCH_ARM_HAVE_TLS_REGISTER := true


TARGET_BOOTLOADER_BOARD_NAME := mione	

ANDROID_COMMON_BUILD_MK := true

BOARD_KERNEL_CMDLINE := console=null androidboot.hardware=mione
BOARD_KERNEL_BASE := 0x40200000
BOARD_KERNEL_PAGESIZE := 2048
BOARD_MKBOOTIMG_ARGS := --ramdisk_offset 0x01200000

# fix this up by examining /proc/mtd on a running device
BOARD_BOOTIMAGE_PARTITION_SIZE := 0x105c0000
BOARD_RECOVERYIMAGE_PARTITION_SIZE := 0x105c0000
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 0x105c0000
BOARD_USERDATAIMAGE_PARTITION_SIZE := 0x105c0000
BOARD_FLASH_BLOCK_SIZE := 131072

#kernel
TARGET_PREBUILT_KERNEL := device/xiaomi/mione_plus/kernel

#twrp
LZMA_RAMDISK_TARGETS                    := boot,recovery
RECOVERY_VARIANT                        := twrp
TW_THEME := portrait_mdpi
TARGET_PREBUILT_RECOVERY_KERNEL         := $(call my-dir)/kernel
TARGET_RECOVERY_PIXEL_FORMAT := "RGBX_8888"
RECOVERY_GRAPHICS_USE_LINELENGTH        := true
TARGET_RECOVERY_FSTAB                   := device/xiaomi/mione_plus/recovery.fstab
TW_USE_MODEL_HARDWARE_ID_FOR_DEVICE_ID  := true
TARGET_RECOVERY_LCD_BACKLIGHT_PATH      := "/sys/class/leds/lcd-backlight/brightness"
TW_NO_CPU_TEMP := true
TW_NEW_ION_HEAP := true
TW_NO_SCREEN_BLANK                      := true
TARGET_RECOVERY_QCOM_RTC_FIX            := true
BOARD_SUPPRESS_SECURE_ERASE             := true
BOARD_SUPPRESS_EMMC_WIPE                := true

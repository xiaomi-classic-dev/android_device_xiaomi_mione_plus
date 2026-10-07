#
# Copyright (C) 2012 The CyanogenMod Project
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

$(call inherit-product, $(SRC_TARGET_DIR)/product/full_base_telephony.mk)

# Copy Bluetooth firmware, since BCM4329 is a BT/WiFi chip
PRODUCT_COPY_FILES += \
    device/xiaomi/mione_plus/firmware/bcm4330.hcd:system/vendor/firmware/bcm4330.hcd \
    device/xiaomi/mione_plus/firmware/bcm4329.hcd:system/vendor/firmware/bcm4329.hcd



# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/handheld_core_hardware.xml:system/vendor/etc/permissions/handheld_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.camera.autofocus.xml:system/vendor/etc/permissions/android.hardware.camera.autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.flash-autofocus.xml:system/vendor/etc/permissions/android.hardware.camera.flash-autofocus.xml \
    frameworks/native/data/etc/android.hardware.camera.front.xml:system/vendor/etc/permissions/android.hardware.camera.front.xml \
    frameworks/native/data/etc/android.hardware.ethernet.xml:system/vendor/etc/permissions/android.hardware.ethernet.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:system/vendor/etc/permissions/android.hardware.location.gps.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:system/vendor/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:system/vendor/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.sensor.proximity.xml:system/vendor/etc/permissions/android.hardware.sensor.proximity.xml \
    frameworks/native/data/etc/android.hardware.sensor.light.xml:system/vendor/etc/permissions/android.hardware.sensor.light.xml \
    frameworks/native/data/etc/android.hardware.sensor.gyroscope.xml:system/vendor/etc/permissions/android.hardware.sensor.gyroscope.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.distinct.xml:system/vendor/etc/permissions/android.hardware.touchscreen.multitouch.distinct.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:system/vendor/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.usb.host.xml:system/vendor/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.software.sip.voip.xml:system/vendor/etc/permissions/android.software.sip.voip.xml \
    frameworks/native/data/etc/android.hardware.sensor.accelerometer.xml:system/vendor/etc/permissions/android.hardware.sensor.accelerometer.xml \
    frameworks/native/data/etc/android.hardware.sensor.compass.xml:system/vendor/etc/permissions/android.hardware.sensor.compass.xml

# Audio
PRODUCT_PACKAGES += \
    audio.a2dp.default \
    audio.r_submix.default \
    audio.usb.default \
    audio.primary.msm8660 \
    libaudioutils \
    libaudio-resampler

# GPS
PRODUCT_PACKAGES += \
    gps.msm8660

# Graphics
PRODUCT_PACKAGES += \
    camera.msm8660 \
    copybit.msm8660 \
    gralloc.msm8660 \
    hwcomposer.msm8660 \
    memtrack.msm8660 \
    android.hardware.light@2.0-service.mione \
    libgenlock \
    libmemalloc \
    liboverlay \
    libqdutils

# Qcom
PRODUCT_PACKAGES += \
    libstlport \
    libstdc++

# OMX
PRODUCT_PACKAGES += \
    libc2dcolorconvert \
    libdivxdrmdecrypt \
    libmm-omxcore \
    libOmxCore \
    libOmxVdec \
    libOmxVenc \
    libOmxAacEnc \
    libOmxAmrEnc \
    libstagefrighthw \
    libOmxQcelp13Enc \
    libOmxEvrcEnc

# HDMI
PRODUCT_PACKAGES += \
    hdmid

# USB
PRODUCT_PACKAGES += \
    com.android.future.usb.accessory

# Filesystem management tools
PRODUCT_PACKAGES += \
    make_ext4fs \
    setup_fs

# Legacy camera HAL
PRODUCT_PACKAGES += \
    camera.msm8660

# IPv6 tethering
PRODUCT_PACKAGES += \
    ebtables \
    ethertypes

# Net
PRODUCT_PACKAGES += \
    libnetcmdiface \
    crda

# WiFi
PRODUCT_PACKAGES += \
    dhcpcd.conf \
    hostapd \
    hostapd_default.conf \
    wpa_supplicant \
    wpa_supplicant.conf

# Bluetooth
PRODUCT_PACKAGES += \
    bt_vendor.conf

# Media configuration
PRODUCT_COPY_FILES += \
    frameworks/av/media/libstagefright/data/media_codecs_google_audio.xml:system/vendor/etc/media_codecs_google_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_telephony.xml:system/vendor/etc/media_codecs_google_telephony.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_video.xml:system/vendor/etc/media_codecs_google_video.xml \
    device/xiaomi/mione_plus/configs/media_codecs.xml:system/vendor/etc/media_codecs.xml \
    device/xiaomi/mione_plus/configs/media_profiles.xml:system/vendor/etc/media_profiles.xml

# audio policy
PRODUCT_COPY_FILES += \
    device/xiaomi/mione_plus/configs/audio_policy.conf:system/vendor/etc/audio_policy.conf

# MSM8660 firmware
PRODUCT_COPY_FILES += \
    device/xiaomi/mione_plus/firmware/leia_pfp_470.fw:system/vendor/firmware/leia_pfp_470.fw \
    device/xiaomi/mione_plus/firmware/leia_pm4_470.fw:system/vendor/firmware/leia_pm4_470.fw \
    device/xiaomi/mione_plus/firmware/vidc_1080p.fw:system/vendor/firmware/vidc_1080p.fw

# Thermal configuration
PRODUCT_PACKAGES += android.hardware.thermal@1.0-service.mione
PRODUCT_COPY_FILES += \
    device/xiaomi/mione_plus/configs/thermald.conf:system/vendor/etc/thermald.conf

# Device uses high-density artwork where available
PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := hdpi

# Common build properties
PRODUCT_PROPERTY_OVERRIDES += \
    com.qc.hardware=true \
    debug.egl.hw=1 \
    debug.mdpcomp.logs=0 \
    debug.sf.hw=1 \
    dev.pm.dyn_samplingrate=1 \
    ro.opengles.version=131072

$(call inherit-product-if-exists, vendor/xiaomi/mione_plus/mione_plus-vendor.mk)

DEVICE_PACKAGE_OVERLAYS += device/xiaomi/mione_plus/overlay

# MSM8660 power policy
PRODUCT_PACKAGES += \
    android.hardware.power@1.0-service.mione

# Translate the shipped CAF v6 RIL contract at the vendor library boundary.
PRODUCT_PACKAGES += libril_mione

# gps.conf
PRODUCT_COPY_FILES += \
    device/xiaomi/mione_plus/configs/gps.conf:system/vendor/etc/gps.conf

# mac support for mione_plus
# credit: huangqiwu
# https://github.com/mirom/android_device_xiaomi_mione_plus/commit/cf62e83ee96d90f0735c56b85fb8e252574c644d
PRODUCT_PACKAGES += \
    libreadmac

# Hostapd (Required for Wi-Fi)
PRODUCT_PACKAGES += \
    hostapd_cli \
    calibrator \
    hostapd

PRODUCT_PACKAGES += libmione_cnd_shim libmione_sensors_shim

# Vendor init configuration and scripts.  MiOne needs a small ramdisk
# bootstrap because /vendor is supplied by /system rather than a partition.
PRODUCT_COPY_FILES += \
    device/xiaomi/mione_plus/rootdir/etc/init.qcom.rc:system/vendor/etc/init/hw/init.qcom.rc \
    device/xiaomi/mione_plus/rootdir/etc/init.qcom.usb.rc:system/vendor/etc/init/hw/init.qcom.usb.rc \
    device/xiaomi/mione_plus/rootdir/etc/init.target.rc:system/vendor/etc/init/hw/init.target.rc \
    device/xiaomi/mione_plus/rootdir/etc/fstab.qcom:system/vendor/etc/fstab.qcom \
    device/xiaomi/mione_plus/rootdir/etc/ueventd.qcom.rc:system/vendor/ueventd.rc \
    device/xiaomi/mione_plus/rootdir/bin/init.qcom.class_core.sh:system/vendor/bin/init.qcom.class_core.sh \
    device/xiaomi/mione_plus/rootdir/bin/init.qcom.class_main.sh:system/vendor/bin/init.qcom.class_main.sh \
    device/xiaomi/mione_plus/rootdir/bin/init.qcom.sh:system/vendor/bin/init.qcom.sh \
    device/xiaomi/mione_plus/rootdir/bin/init.qcom.usb.sh:system/vendor/bin/init.qcom.usb.sh \
    device/xiaomi/mione_plus/rootdir/bin/init.qcom.post_boot.sh:system/vendor/bin/init.qcom.post_boot.sh \
    device/xiaomi/mione_plus/rootdir/bin/init.qcom.efs.sync.sh:system/vendor/bin/init.qcom.efs.sync.sh \
    device/xiaomi/mione_plus/ramdisk/init.qcom.bootstrap.rc:root/init.qcom.rc \
    device/xiaomi/mione_plus/ramdisk/fstab.qcom.early:root/fstab.qcom.early \
    device/xiaomi/mione_plus/rootdir/etc/fstab.qcom:root/fstab.qcom \
    device/xiaomi/mione_plus/rootdir/etc/ueventd.qcom.rc:root/ueventd.qcom.rc

# WiFi
PRODUCT_PACKAGES += \
    dhcpcd.conf \
    hostapd \
    hostapd_default.conf \
    wpa_supplicant \
    wpa_supplicant.conf

# Bluetooth
PRODUCT_PACKAGES += \
    bt_vendor.conf \
    mione_bdaddr

# Input device config
PRODUCT_COPY_FILES += \
    device/xiaomi/mione_plus/configs/mxt224_ts_input.idc:system/vendor/usr/idc/synaptics_rmi4_i2c.idc \
    device/xiaomi/mione_plus/configs/mxt224_ts_input.idc:system/vendor/usr/idc/ft5x0x.idc \
    device/xiaomi/mione_plus/configs/mxt224_ts_input.idc:system/vendor/usr/idc/sensor00fn11.idc \
    device/xiaomi/mione_plus/configs/mxt224_ts_input.idc:system/vendor/usr/idc/sensor00fn54.idc \
    device/xiaomi/mione_plus/configs/mxt224_ts_input.idc:system/vendor/usr/idc/mxt224_ts_input.idc \
    device/xiaomi/mione_plus/configs/mxt224_ts_input.idc:system/vendor/usr/idc/mXT-touch.idc

# Prebuilt modules belong only to the matching prebuilt kernel. Source kernel
# builds install their own modules through build/core/tasks/kernel.mk.
ifneq ($(TARGET_PREBUILT_KERNEL),)
PRODUCT_COPY_FILES += $(shell \
    find device/xiaomi/mione_plus/prebuilt -name '*.ko' \
    | sed -r 's/^\/?(.*\/)([^/ ]+)$$/\1\2:system\/vendor\/lib\/modules\/\2/' \
    | tr '\n' ' ')
endif

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.telephony.gsm.xml:system/vendor/etc/permissions/android.hardware.telephony.gsm.xml \
    frameworks/native/data/etc/android.hardware.telephony.cdma.xml:system/vendor/etc/permissions/android.hardware.telephony.cdma.xml \
    frameworks/native/data/etc/android.hardware.touchscreen.multitouch.jazzhand.xml:system/vendor/etc/permissions/android.hardware.touchscreen.multitouch.jazzhand.xml

# The BCM4329 firmware shipped for MiOne Plus returns BCME_UNSUPPORTED for
# P2P. BCM4330 firmware must not be used on it. A separately validated BCM4330
# product can opt in to this feature and its matching firmware configuration.
ifeq ($(BOARD_MIONE_WIFI_DIRECT),true)
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:system/vendor/etc/permissions/android.hardware.wifi.direct.xml
endif

PRODUCT_PROPERTY_OVERRIDES += \
    dalvik.vm.dexopt-flags=m=y \
    persist.sys.timezone=Asia/Shanghai

# Start USB/root ADB in userdebug without switching to an eng build.
ifeq ($(TARGET_BUILD_VARIANT),userdebug)
PRODUCT_DEFAULT_PROPERTY_OVERRIDES += \
    persist.sys.usb.config=adb \
    lineage.service.adb.root=1

# Trust the build host's public key so unattended debugging needs no prompt.
# adbd reads /adb_keys while retaining normal RSA authentication.
PRODUCT_COPY_FILES += \
    device/xiaomi/mione_plus/configs/adb_keys:root/adb_keys

# Use RGB video uploads while keeping WebView GPU rasterization/compositing.
PRODUCT_COPY_FILES += \
    device/xiaomi/mione_plus/configs/webview-command-line:system/vendor/etc/webview-command-line \
    device/xiaomi/mione_plus/configs/init.mione.webview.rc:system/vendor/etc/init/init.mione.webview.rc
endif

# xiaomi mione wifi config
$(call inherit-product, device/xiaomi/mione_plus/mione_bcm43xx.mk)

# dalvik tweak
$(call inherit-product, frameworks/native/build/phone-hdpi-512-dalvik-heap.mk)

# Oreo adapters retain the device's source-built legacy HALs.
PRODUCT_PACKAGES += \
    android.hardware.audio@2.0-impl \
    android.hardware.audio.effect@2.0-impl \
    android.hardware.bluetooth@1.0-impl \
    camera.device@1.0-impl-legacy \
    android.hardware.camera.provider@2.4-impl-legacy \
    android.hardware.gnss@1.0-impl \
    android.hardware.gnss@1.0-service.mione \
    android.hardware.graphics.allocator@2.0-impl \
    android.hardware.graphics.allocator@2.0-service \
    android.hardware.graphics.mapper@2.0-impl \
    android.hardware.graphics.composer@2.1-impl \
    android.hardware.graphics.composer@2.1-service \
    android.hardware.keymaster@3.0-impl \
    android.hardware.keymaster@3.0-service \
    android.hardware.memtrack@1.0-impl \
    sensors.msm8660 \
    android.hardware.sensors@1.0-impl \
    vibrator.default \
    android.hardware.vibrator@1.0-impl \
    android.hardware.wifi@1.0-impl \
    android.hardware.wifi@1.0-service \
    android.hardware.wifi.supplicant@1.0 \
    libbt-vendor \
    wificond

PRODUCT_COPY_FILES += \
    device/xiaomi/mione_plus/seccomp_policy/mediacodec.policy:system/vendor/etc/seccomp_policy/mediacodec.policy \
    device/xiaomi/mione_plus/seccomp_policy/mediaextractor.policy:system/vendor/etc/seccomp_policy/mediaextractor.policy

# Preserve the legacy Wi-Fi loader capability in images and OTA metadata.
PRODUCT_PACKAGES += fs_config_files

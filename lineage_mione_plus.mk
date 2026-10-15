
# Release name
PRODUCT_RELEASE_NAME := mione_plus

TARGET_BOOTANIMATION_NAME := vertical-480x854

# Inherit some common LineageOS stuff.
$(call inherit-product, vendor/lineage/config/common_full_phone.mk)

# Inherit device configuration
$(call inherit-product, device/xiaomi/mione_plus/device_mione_plus.mk)

# Setup device specific product configuration.
PRODUCT_DEVICE := mione_plus
PRODUCT_NAME := lineage_mione_plus
PRODUCT_BRAND := Xiaomi
PRODUCT_MODEL := MI-ONE Plus
PRODUCT_MANUFACTURER := Xiaomi

PRODUCT_CHARACTERISTICS := phone

# Set build fingerprint / ID / Product Name etc.
PRODUCT_BUILD_PROP_OVERRIDES += \
    BuildDesc="mione_plus-userdebug 4.1.2 JZO54K QDS84 test-keys" \
    BuildFingerprint=Xiaomi/mione_plus/mione_plus:4.1.2/JZO54K/QDS84:userdebug/test-keys \
    DeviceName=mione_plus \
    DeviceProduct=mione_plus \
    SystemDevice=mione_plus \
    SystemName=mione_plus

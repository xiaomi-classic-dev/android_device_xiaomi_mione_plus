LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_DEVICE),mione_plus)
mione_boot_image_profile := $(PRODUCT_DEX_PREOPT_BOOT_IMAGE_PROFILE_LOCATION)
mione_boot_image_profile_inputs := frameworks/base/config/boot-image-profile.txt $(LOCAL_PATH)/wallpaper-profile.txt

$(mione_boot_image_profile): PRIVATE_MIONE_BOOT_IMAGE_PROFILES := $(mione_boot_image_profile_inputs)
$(mione_boot_image_profile): $(mione_boot_image_profile_inputs)
	@echo "MiOne boot image profile: $@"
	$(hide) mkdir -p $(dir $@)
	$(hide) cat $(PRIVATE_MIONE_BOOT_IMAGE_PROFILES) > $@.tmp
	$(hide) mv $@.tmp $@
endif

# Device-owned boot ramdisk prelude for LVM, preserving stock Android init.
MIONE_LVM_FILES := $(wildcard device/xiaomi/mione_plus/lvm/*) device/xiaomi/mione_plus/lvm/prebuilt/lvm
MIONE_LVM_RAMDISK := $(PRODUCT_OUT)/mione-lvm-ramdisk.img
MIONE_LVM_BOOT_ARGS = $(subst --ramdisk $(INSTALLED_RAMDISK_TARGET),--ramdisk $(MIONE_LVM_RAMDISK),$(INTERNAL_BOOTIMAGE_ARGS))

$(INSTALLED_BOOTIMAGE_TARGET): $(MKBOOTIMG) $(MKBOOTFS) $(INTERNAL_BOOTIMAGE_FILES) $(BOOTIMAGE_EXTRA_DEPS) $(MIONE_LVM_FILES)
	$(hide) python3 device/xiaomi/mione_plus/lvm/prepare-boot.py $(TARGET_RAMDISK_OUT) device/xiaomi/mione_plus/lvm
	$(hide) $(MKBOOTFS) -d $(TARGET_OUT) $(TARGET_RAMDISK_OUT) | $(COMPRESSION_COMMAND) > $(MIONE_LVM_RAMDISK)
	$(hide) $(MKBOOTIMG) $(MIONE_LVM_BOOT_ARGS) $(INTERNAL_MKBOOTIMG_VERSION_ARGS) $(BOARD_MKBOOTIMG_ARGS) --output $@
	$(hide) $(call assert-max-image-size,$@,$(BOARD_BOOTIMAGE_PARTITION_SIZE))

.PHONY: bootimage-nodeps
bootimage-nodeps: $(INSTALLED_BOOTIMAGE_TARGET)

# An automatic recovery refresh must also preserve LVM boot activation.
MIONE_LVM_RECOVERY_RAMDISK := $(PRODUCT_OUT)/mione-lvm-recovery-ramdisk.img
$(INSTALLED_RECOVERYIMAGE_TARGET): $(recoveryimage-deps) $(RECOVERYIMAGE_EXTRA_DEPS) $(INSTALLED_BOOTIMAGE_TARGET) $(MIONE_LVM_FILES)
	$(hide) python3 device/xiaomi/mione_plus/lvm/prepare-recovery.py $(TARGET_RECOVERY_ROOT_OUT) $(TARGET_RAMDISK_OUT) device/xiaomi/mione_plus/lvm
	$(hide) $(MKBOOTFS) -d $(TARGET_OUT) $(TARGET_RECOVERY_ROOT_OUT) | $(COMPRESSION_COMMAND) > $(MIONE_LVM_RECOVERY_RAMDISK)
	$(hide) $(MKBOOTIMG) $(subst --ramdisk $(recovery_ramdisk),--ramdisk $(MIONE_LVM_RECOVERY_RAMDISK),$(INTERNAL_RECOVERYIMAGE_ARGS)) $(INTERNAL_MKBOOTIMG_VERSION_ARGS) $(BOARD_MKBOOTIMG_ARGS) --output $@
	$(hide) $(call assert-max-image-size,$@,$(BOARD_RECOVERYIMAGE_PARTITION_SIZE))

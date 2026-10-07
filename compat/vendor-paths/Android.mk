# The Qualcomm V6 graphics blobs dlopen these absolute system paths.
# Store one vendor copy and preserve the ABI paths as symlinks.
MIONE_VENDOR_COMPAT_LIBS := \
    egl/eglsubAndroid.so \
    egl/libEGL_adreno200.so \
    egl/libGLESv1_CM_adreno200.so \
    egl/libGLESv2_adreno200.so \
    egl/libGLESv2S3D_adreno200.so \
    egl/libq3dtools_adreno200.so \
    libOpenCL.so \
    libOpenVG.so \
    libsc-a2xx.so

define mione-vendor-compat-link
$(TARGET_OUT_SHARED_LIBRARIES)/$(1): $(TARGET_OUT_VENDOR_SHARED_LIBRARIES)/$(1)
	@echo "MiOne vendor compatibility link: $$@"
	$(hide) mkdir -p $$(dir $$@)
	$(hide) ln -sf /vendor/lib/$(1) $$@
endef

$(foreach lib,$(MIONE_VENDOR_COMPAT_LIBS),$(eval $(call mione-vendor-compat-link,$(lib))))
ALL_DEFAULT_INSTALLED_MODULES += $(addprefix $(TARGET_OUT_SHARED_LIBRARIES)/,$(MIONE_VENDOR_COMPAT_LIBS))

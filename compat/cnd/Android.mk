LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libmione_cnd_shim
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := cnd_shim.c parcel_shim.cpp
LOCAL_C_INCLUDES := external/icu/icu4c/source/common
LOCAL_SHARED_LIBRARIES := libicuuc libbinder
LOCAL_CFLAGS := -Wall -Wextra -Werror
include $(BUILD_SHARED_LIBRARY)

# Explicit test target; not installed in the product.
include $(CLEAR_VARS)
LOCAL_MODULE := mione_cnd_shim_test
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := tests
LOCAL_SRC_FILES := cnd_shim_test.c
LOCAL_C_INCLUDES := external/icu/icu4c/source/common
LOCAL_SHARED_LIBRARIES := libmione_cnd_shim libicuuc
LOCAL_CFLAGS := -Wall -Wextra -Werror
include $(BUILD_EXECUTABLE)

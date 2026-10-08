LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libmione_fake_ril
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := tests
LOCAL_SRC_FILES := fake_ril.c
LOCAL_HEADER_LIBRARIES := ril_headers
LOCAL_CFLAGS := -DRIL_SHLIB -Wall -Wextra -Werror
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := libmione_ril_test
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := tests
LOCAL_SRC_FILES := ../ril_mione.c
LOCAL_HEADER_LIBRARIES := ril_headers
LOCAL_CFLAGS := -DRIL_SHLIB -Wall -Wextra -Werror \
    -DMIONE_VENDOR_RIL=\"/data/local/tmp/mione-userspace/libmione_fake_ril.so\"
LOCAL_SHARED_LIBRARIES := libdl liblog
include $(BUILD_SHARED_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mione_ril_probe
LOCAL_VENDOR_MODULE := true
LOCAL_MODULE_TAGS := tests
LOCAL_SRC_FILES := ril_probe.c
LOCAL_HEADER_LIBRARIES := ril_headers
LOCAL_CFLAGS := -DRIL_SHLIB -Wall -Wextra -Werror
LOCAL_SHARED_LIBRARIES := libdl
include $(BUILD_EXECUTABLE)

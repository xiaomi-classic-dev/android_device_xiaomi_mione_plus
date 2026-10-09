LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES:= xdr.c rpc.c svc.c clnt.c ops.c svc_clnt_common.c

LOCAL_C_INCLUDES:=$(LOCAL_PATH)

LOCAL_CFLAGS:= -fno-short-enums 

LOCAL_CFLAGS+=-DRPC_OFFSET=0
#LOCAL_CFLAGS+=-DDEBUG -DVERBOSE



LOCAL_MODULE:= librpc
LOCAL_VENDOR_MODULE := true

LOCAL_MODULE_TAGS := optional

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := mione_rpc_headers
LOCAL_VENDOR_MODULE := true
LOCAL_EXPORT_C_INCLUDE_DIRS := $(LOCAL_PATH)
include $(BUILD_HEADER_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE := librpc
LOCAL_VENDOR_MODULE := true
LOCAL_SHARED_LIBRARIES := liblog libcutils libpower
LOCAL_WHOLE_STATIC_LIBRARIES := librpc

# LOCAL_PRELINK_MODULE := false
include $(BUILD_SHARED_LIBRARY)

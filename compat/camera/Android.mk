LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE := libmione_camera_unwind_shim
LOCAL_VENDOR_MODULE := true
LOCAL_MULTILIB := 32
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := Unwind.cpp
LOCAL_CFLAGS := -Wall -Wextra -Werror -fno-unwind-tables -fno-asynchronous-unwind-tables
LOCAL_CPPFLAGS := -fno-exceptions -fno-rtti
# This single C entry point has no constructors or destructors. Avoid pulling
# ARM unwind personalities back in through crtbegin or libgcc.
LOCAL_NO_CRT := true
LOCAL_NO_LIBGCC := true
LOCAL_CXX_STL := none
include $(BUILD_SHARED_LIBRARY)

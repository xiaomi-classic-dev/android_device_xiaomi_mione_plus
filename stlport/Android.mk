# Build the legacy Adreno dependency from the retained NDK STLport sources.
LOCAL_PATH := $(call my-dir)
mione_stlport_sources := ndk/sources/cxx-stl/stlport

include $(CLEAR_VARS)
LOCAL_MODULE := libstlport
LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := \
    src/dll_main.cpp \
    src/fstream.cpp \
    src/strstream.cpp \
    src/sstream.cpp \
    src/ios.cpp \
    src/stdio_streambuf.cpp \
    src/istream.cpp \
    src/ostream.cpp \
    src/codecvt.cpp \
    src/collate.cpp \
    src/ctype.cpp \
    src/monetary.cpp \
    src/num_get.cpp \
    src/num_put.cpp \
    src/num_get_float.cpp \
    src/num_put_float.cpp \
    src/numpunct.cpp \
    src/time_facets.cpp \
    src/messages.cpp \
    src/locale.cpp \
    src/locale_impl.cpp \
    src/locale_catalog.cpp \
    src/facets_byname.cpp \
    src/complex.cpp \
    src/complex_io.cpp \
    src/complex_trig.cpp \
    src/string.cpp \
    src/bitset.cpp \
    src/allocators.cpp \
    src/c_locale.c \
    src/cxa.c
LOCAL_SRC_FILES := $(addprefix ../../../../$(mione_stlport_sources)/,$(LOCAL_SRC_FILES)) iostream_compat.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/include $(mione_stlport_sources)/stlport $(mione_stlport_sources)/src bionic
LOCAL_CFLAGS := -D_GNU_SOURCE
LOCAL_CPPFLAGS := -fuse-cxa-atexit
LOCAL_CXX_STL := libstdc++
include $(BUILD_SHARED_LIBRARY)

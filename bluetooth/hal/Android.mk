LOCAL_PATH := $(call my-dir)

# Compile stock HIDL entry points with only the device-owned H4 compatibility.
include $(CLEAR_VARS)
LOCAL_MODULE := android.hardware.bluetooth@1.0-impl.mione
LOCAL_MODULE_RELATIVE_PATH := hw
LOCAL_VENDOR_MODULE := true
LOCAL_MULTILIB := 32
LOCAL_OVERRIDES_MODULES := android.hardware.bluetooth@1.0-impl
LOCAL_SRC_FILES := \
    ../../../../../hardware/interfaces/bluetooth/1.0/default/bluetooth_hci.cc \
    ../../../../../hardware/interfaces/bluetooth/1.0/default/bluetooth_address.cc \
    ../../../../../hardware/interfaces/bluetooth/1.0/default/hci_protocol.cc \
    ../../../../../hardware/interfaces/bluetooth/1.0/default/hci_packetizer.cc \
    ../../../../../hardware/interfaces/bluetooth/1.0/default/mct_protocol.cc \
    vendor_interface.cc \
    h4_protocol.cc
LOCAL_C_INCLUDES := hardware/interfaces/bluetooth/1.0/default
LOCAL_CPP_EXTENSION := .cc
LOCAL_CPP_STD := c++17
LOCAL_SHARED_LIBRARIES := android.hardware.bluetooth@1.0 libbase libcutils libhardware libhidlbase liblog libutils
LOCAL_STATIC_LIBRARIES := android.hardware.bluetooth-async
include $(BUILD_SHARED_LIBRARY)

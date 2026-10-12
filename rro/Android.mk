# Copyright (C) 2026 The LineageOS Project
# SPDX-License-Identifier: Apache-2.0

LOCAL_PATH := $(call my-dir)

ifeq ($(TARGET_DEVICE),mione_plus)
include $(CLEAR_VARS)
LOCAL_MODULE := mione-framework-overlays
# The platform aggregate has unconditional navigation install dependencies,
# even when the hardware-key RRO overrides those individual packages.
LOCAL_REQUIRED_MODULES := \
    DisplayCutoutEmulationCornerOverlay \
    DisplayCutoutEmulationDoubleOverlay \
    DisplayCutoutEmulationHoleOverlay \
    DisplayCutoutEmulationTallOverlay \
    DisplayCutoutEmulationWaterfallOverlay \
    FontNotoSerifSourceOverlay \
    preinstalled-packages-platform-overlays.xml
include $(BUILD_PHONY_PACKAGE)
endif

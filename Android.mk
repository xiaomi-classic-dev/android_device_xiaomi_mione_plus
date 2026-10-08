# Copyright (C) 2010 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

MIONE_DEVICE_PATH := $(call my-dir)

ifeq ($(TARGET_BOOTLOADER_BOARD_NAME),mione)
ifneq ($(BUILD_WITHOUT_VENDOR),true)
# Android 10's hardware/qcom parent excludes the standalone CAF trees.
include $(call project-path-for,qcom-audio)/Android.mk
include $(call project-path-for,qcom-display)/Android.mk
include $(call project-path-for,qcom-media)/Android.mk
endif
include $(call all-makefiles-under,$(MIONE_DEVICE_PATH))
endif

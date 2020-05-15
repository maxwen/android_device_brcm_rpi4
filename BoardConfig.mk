#
# Copyright (C) 2020 The LineageOS Project
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
#

DEVICE_PATH := device/brcm/rpi4

# Platform
TARGET_NO_BOOTLOADER := true
TARGET_NO_RECOVERY := true
TARGET_BOARD_PLATFORM := bcm2711

TARGET_ARCH := arm
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_ABI := armeabi-v7a
TARGET_CPU_ABI2 := armeabi
TARGET_CPU_VARIANT := generic

USE_OPENGL_RENDERER := true
TARGET_USES_HWC2 := true
BOARD_GPU_DRIVERS := v3d
TARGET_USES_64_BIT_BINDER := true
BOARD_USES_DRM_GRALLOC := true

#wifi
BOARD_WLAN_DEVICE := bcmdhd
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
BOARD_HOSTAPD_PRIVATE_LIB   := lib_driver_cmd_bcmdhd
WPA_SUPPLICANT_VERSION := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER := NL80211

# Kernel
BOARD_KERNEL_IMAGE_NAME := zImage
TARGET_KERNEL_SOURCE := kernel/arpi
TARGET_KERNEL_CONFIG := omni_rpi4_defconfig

TARGET_COPY_OUT_VENDOR := vendor
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_SYSTEMIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 1073741824 # 1G
BOARD_VENDORIMAGE_PARTITION_SIZE := 268435456 # 256M
TARGET_USERIMAGES_SPARSE_EXT_DISABLED := true
TARGET_USERIMAGES_USE_EXT4 := true

USE_XML_AUDIO_POLICY_CONF := 1
DEVICE_MANIFEST_FILE := $(DEVICE_PATH)/manifest.xml

USE_CAMERA_STUB := true

BOARD_SEPOLICY_DIRS += \
    $(DEVICE_PATH)/sepolicy

TARGET_SYSTEM_PROP := $(DEVICE_PATH)/system.prop
TARGET_INIT_VENDOR_LIB := libinit_rpi4
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(DEVICE_PATH)/bluetooth

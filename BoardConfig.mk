#
# Copyright (C) 2020 The OmniROM Project
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
TARGET_NO_CHARGER := true
TARGET_BOARD_PLATFORM := bcm2711

BOARD_VNDK_VERSION := current
PRODUCT_FULL_TREBLE_OVERRIDE := true

TARGET_ARCH := arm64
TARGET_ARCH_VARIANT := armv8-a
TARGET_CPU_VARIANT := cortex-a72
TARGET_CPU_ABI := arm64-v8a
TARGET_CPU_ABI2 :=

TARGET_2ND_ARCH := arm
TARGET_2ND_ARCH_VARIANT := armv7-a-neon
TARGET_2ND_CPU_VARIANT := cortex-a72
TARGET_2ND_CPU_ABI := armeabi-v7a
TARGET_2ND_CPU_ABI2 := armeabi

USE_OPENGL_RENDERER := true
TARGET_USES_HWC2 := true
BOARD_GPU_DRIVERS := v3d kmsro
TARGET_USES_64_BIT_BINDER := true

#BOARD_USES_DRM_GRALLOC := true
TARGET_USE_V3D_KMSRO := true

#wifi
BOARD_WLAN_DEVICE := bcmdhd
BOARD_WPA_SUPPLICANT_PRIVATE_LIB := lib_driver_cmd_bcmdhd
BOARD_HOSTAPD_PRIVATE_LIB   := lib_driver_cmd_bcmdhd
BOARD_HOSTAPD_DRIVER := NL80211
WPA_SUPPLICANT_VERSION := VER_0_8_X
BOARD_WPA_SUPPLICANT_DRIVER := NL80211

# Kernel
BOARD_KERNEL_IMAGE_NAME := Image
BOARD_KERNEL_SEPARATED_DTBO := false
TARGET_KERNEL_SOURCE := kernel/brcm/arpi
TARGET_KERNEL_CONFIG := omni_rpi4_defconfig
TARGET_KERNEL_ARCH := arm64
TARGET_KERNEL_HEADER_ARCH := arm64
TARGET_KERNEL_CLANG_COMPILE := true

TARGET_COPY_OUT_PRODUCT := system/product
TARGET_COPY_OUT_SYSTEM_EXT := system/system_ext
TARGET_COPY_OUT_VENDOR := vendor
BOARD_VENDORIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_SYSTEMIMAGE_FILE_SYSTEM_TYPE := ext4
BOARD_SYSTEMIMAGE_PARTITION_SIZE := 2147483648 # 2G
BOARD_VENDORIMAGE_PARTITION_SIZE := 268435456 # 256M
TARGET_USERIMAGES_SPARSE_EXT_DISABLED := true
TARGET_USERIMAGES_USE_EXT4 := true
BOARD_BOOTIMAGE_PARTITION_SIZE := 268435456 # 256M

USE_XML_AUDIO_POLICY_CONF := 1
DEVICE_MANIFEST_FILE := $(DEVICE_PATH)/manifest.xml
DEVICE_PRODUCT_COMPATIBILITY_MATRIX_FILE := $(DEVICE_PATH)/compatibility_matrix.device.xml

BOARD_SEPOLICY_DIRS += \
    $(DEVICE_PATH)/sepolicy

TARGET_SYSTEM_PROP := $(DEVICE_PATH)/system.prop
TARGET_INIT_VENDOR_LIB := //$(DEVICE_PATH):libinit_rpi4
TARGET_VOLD_VENDOR_LIB := //$(DEVICE_PATH):libinit_rpi4
BOARD_BLUETOOTH_BDROID_BUILDCFG_INCLUDE_DIR := $(DEVICE_PATH)/bluetooth/cfg

BUILD_FINGERPRINT := Raspberry/omni_rpi4/rpi4:10/QKQ1.190716.003/1910071200:userdebug/release-keys

# recovery
# change to false when building recoveryimage
TARGET_NO_RECOVERY := true
BOARD_USES_RECOVERY_AS_BOOT := true
TARGET_RECOVERY_FSTAB := $(DEVICE_PATH)/recovery/recovery.fstab
TARGET_RECOVERY_PIXEL_FORMAT := "RGB_565"
TARGET_RECOVERY_FORCE_PIXEL_FORMAT := "RGB_565"
TW_BRIGHTNESS_PATH := "/sys/class/backlight/rpi_backlight/brightness"
TW_EXCLUDE_MTP := true
TW_INCLUDE_CRYPTO := true
TW_NO_BATT_PERCENT := true
TW_NO_REBOOT_BOOTLOADER := true
TW_NO_REBOOT_RECOVERY := true
TW_NO_SCREEN_TIMEOUT := true
TW_THEME := landscape_hdpi
TW_USE_TOOLBOX := true
TWRP_INCLUDE_LOGCAT := true
TARGET_USES_LOGD := true
TW_EXTERNAL_STORAGE_PATH := "/usb"

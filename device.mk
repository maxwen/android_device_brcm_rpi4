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

$(call inherit-product, frameworks/native/build/tablet-7in-xhdpi-2048-dalvik-heap.mk)
$(call inherit-product, vendor/brcm/rpi4/rpi4-vendor.mk)

$(call inherit-product, device/brcm/rpi-common/device-common.mk)

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/bluetooth.bin/android.hardware.bluetooth@1.0-service.rpi4:$(TARGET_COPY_OUT_VENDOR)/bin/hw/android.hardware.bluetooth@1.0-service.rpi4 \
    $(DEVICE_PATH)/bluetooth.bin/android.hardware.bluetooth@1.0-service.rpi4.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/android.hardware.bluetooth@1.0-service.rpi4.rc

#PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/light/android.hardware.light@2.0-service.rpi4:$(TARGET_COPY_OUT_VENDOR)/bin/hw/android.hardware.light@2.0-service.rpi4 \
    $(DEVICE_PATH)/light/android.hardware.light@2.0-service.rpi4.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/android.hardware.light@2.0-service.rpi4.rc

DEVICE_FRAMEWORK_MANIFEST_FILE += \
    system/libhidl/vintfdata/manifest_healthd_exclude.xml

#PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/health/android.hardware.health@2.0-service.rpi4:$(TARGET_COPY_OUT_VENDOR)/bin/hw/android.hardware.health@2.0-service.rpi4 \
    $(DEVICE_PATH)/health/android.hardware.health@2.0-service.rpi4.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/android.hardware.health@2.0-service.rpi4.rc

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/init.rpi4.rc:root/init.rpi4.rc \
    $(DEVICE_PATH)/init.rpi4.usb.rc:root/init.rpi4.usb.rc \
    $(DEVICE_PATH)/ueventd.rpi4.rc:root/ueventd.rpi4.rc \
    $(DEVICE_PATH)/fstab.rpi4:root/fstab.rpi4

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/Generic.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/Generic.kl

PRODUCT_PACKAGES += \
    audio.primary.rpi4 \
    memtrack.rpi4 \
    gatekeeper.rpi4 \
    libyuv

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

include frameworks/native/build/tablet-7in-hdpi-1024-dalvik-heap.mk

PRODUCT_AAPT_CONFIG := normal
PRODUCT_AAPT_PREF_CONFIG := mdpi

# Audio
PRODUCT_PACKAGES += \
    android.hardware.audio@2.0-impl \
    android.hardware.audio@2.0-service \
    android.hardware.audio.effect@2.0-impl \
    audio.r_submix.default \
    audio.usb.default

#PRODUCT_PACKAGES += \
    android.hardware.graphics.allocator@2.0-service.rpi4 \
    android.hardware.graphics.mapper@2.0-impl.rpi4 \
    android.hardware.graphics.composer@2.1-service.rpi4 \
    gralloc.rpi4

PRODUCT_PACKAGES += \
    android.hardware.graphics.allocator@2.0-impl \
    android.hardware.graphics.allocator@2.0-service \
    android.hardware.graphics.composer@2.1-impl \
    android.hardware.graphics.composer@2.1-service \
    android.hardware.graphics.mapper@2.0-impl \
    hwcomposer.drm \
    gralloc.gbm

PRODUCT_PACKAGES += \
    android.hardware.memtrack@1.0-impl \
    android.hardware.memtrack@1.0-service

# egl
PRODUCT_PACKAGES += \
    libGLES_mesa

#PRODUCT_PACKAGES += \
    libEGL_swiftshader \
    libGLESv1_CM_swiftshader \
    libGLESv2_swiftshader

# Keymaster HAL
PRODUCT_PACKAGES += \
    android.hardware.keymaster@3.0-impl \
    android.hardware.keymaster@3.0-service

PRODUCT_PACKAGES += \
    android.hardware.gatekeeper@1.0-impl \
    android.hardware.gatekeeper@1.0-service.software

# DRM
PRODUCT_PACKAGES += \
    android.hardware.drm@1.0-impl \
    android.hardware.drm@1.0-service

# camera
PRODUCT_PACKAGES += \
    android.hardware.camera.provider@2.5-external-service

PRODUCT_PACKAGES += \
    android.hardware.configstore@1.1-service \
    vndservicemanager

# gps
PRODUCT_PACKAGES += \
    android.hardware.gnss@1.0-impl \
    android.hardware.gnss@1.0-service

PRODUCT_PACKAGES += \
    android.hardware.health@2.1-service \
    android.hardware.health@2.1-impl \
    android.hardware.health.storage@1.0-service

PRODUCT_PACKAGES += \
    android.hardware.power@1.0-impl \
    android.hardware.power@1.0-service

# Permissions
PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.usb.host.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.host.xml \
    frameworks/native/data/etc/android.hardware.usb.accessory.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.usb.accessory.xml \
    frameworks/native/data/etc/android.hardware.wifi.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.xml \
    frameworks/native/data/etc/tablet_core_hardware.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/tablet_core_hardware.xml \
    frameworks/native/data/etc/android.hardware.ethernet.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.ethernet.xml \
    frameworks/native/data/etc/android.hardware.bluetooth_le.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth_le.xml \
    frameworks/native/data/etc/android.hardware.bluetooth.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.bluetooth.xml \
    frameworks/native/data/etc/android.hardware.location.gps.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.location.gps.xml

PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.camera.external.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.camera.external.xml \
    $(DEVICE_PATH)/external_camera_config.xml:$(TARGET_COPY_OUT_VENDOR)/etc/external_camera_config.xml

#PRODUCT_COPY_FILES += \
    frameworks/native/data/etc/android.hardware.wifi.direct.xml:$(TARGET_COPY_OUT_VENDOR)/etc/permissions/android.hardware.wifi.direct.xml \

# Wifi
PRODUCT_PACKAGES += \
    android.hardware.wifi@1.0-service \
    hostapd \
    hostapd_cli \
    wpa_supplicant \
    wpa_cli \
    wpa_supplicant.conf \
    libkeystore-wifi-hidl \
    libkeystore-engine-wifi-hidl

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/wifi/wpa_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/wpa_supplicant_overlay.conf \
    $(DEVICE_PATH)/wifi/p2p_supplicant_overlay.conf:$(TARGET_COPY_OUT_VENDOR)/etc/wifi/p2p_supplicant_overlay.conf

PRODUCT_COPY_FILES += \
     $(DEVICE_PATH)/hostapd/hostapd_default.conf:$(TARGET_COPY_OUT_VENDOR)/etc/hostapd/hostapd_default.conf \
     $(DEVICE_PATH)/hostapd/hostapd.accept:$(TARGET_COPY_OUT_VENDOR)/etc/hostapd/hostapd.accept \
     $(DEVICE_PATH)/hostapd/hostapd.deny:$(TARGET_COPY_OUT_VENDOR)/etc/hostapd/hostapd.deny

# bluetooth
PRODUCT_PACKAGES += \
    android.hardware.bluetooth@1.0-service.rpi4 \
    btuart

# USB HAL
PRODUCT_PACKAGES += \
    android.hardware.usb@1.0-service.basic

# media configurations
PRODUCT_COPY_FILES += \
    frameworks/av/media/libeffects/data/audio_effects.conf:$(TARGET_COPY_OUT_VENDOR)/etc/audio_effects.conf \
    frameworks/av/services/audiopolicy/config/a2dp_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/a2dp_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/usb_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/usb_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/r_submix_audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/r_submix_audio_policy_configuration.xml \
    frameworks/av/services/audiopolicy/config/default_volume_tables.xml:$(TARGET_COPY_OUT_VENDOR)/etc/default_volume_tables.xml \
    $(DEVICE_PATH)/audio_policy_volumes.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_volumes.xml \
    $(DEVICE_PATH)/audio_policy_configuration.xml:$(TARGET_COPY_OUT_VENDOR)/etc/audio_policy_configuration.xml

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/media_codecs.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2_video.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2_video.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2_audio.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2_audio.xml \
    frameworks/av/media/libstagefright/data/media_codecs_google_c2_tv.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_codecs_google_c2_tv.xml \
    $(DEVICE_PATH)/media_profiles_V1_0.xml:$(TARGET_COPY_OUT_VENDOR)/etc/media_profiles_V1_0.xml

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/init.vendor.rpi4.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.rpi4.rc \
    $(DEVICE_PATH)/init.rpi4.usb.rc:$(TARGET_COPY_OUT_VENDOR)/etc/init/hw/init.rpi4.usb.rc \
    $(DEVICE_PATH)/ueventd.rpi4.rc:root/ueventd.rpi4.rc \
    $(DEVICE_PATH)/init.system.rpi4.rc:root/init.rpi4.rc \
    $(DEVICE_PATH)/fstab.rpi4:$(TARGET_COPY_OUT_VENDOR)/etc/fstab.rpi4 \
    $(DEVICE_PATH)/fstab.rpi4:$(TARGET_COPY_OUT_RAMDISK)/fstab.rpi4

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/Generic.kl:$(TARGET_COPY_OUT_VENDOR)/usr/keylayout/Generic.kl

PRODUCT_PACKAGES += \
    audio.primary.rpi4 \
    memtrack.rpi4 \
    gps.rpi4 \
    power.rpi4

#PRODUCT_PACKAGES += \
    gatekeeper.rpi4 \

PRODUCT_PACKAGES += \
    libinit_rpi4

PRODUCT_PACKAGES += \
    ims-ext-common \
    ims_ext_common.xml \
    qti-telephony-hidl-wrapper \
    qti_telephony_hidl_wrapper.xml \
    qti-telephony-utils \
    qti_telephony_utils.xml

PRODUCT_PACKAGES += \
    Provision \
    DeviceParts \
    Terminal \
    webview

PRODUCT_PACKAGES += \
    htop \
    zip

PRODUCT_PACKAGES += \
    gpiodetect \
    gpiofind \
    gpioget \
    gpioinfo \
    gpiomon \
    gpioset \
    gpiod \
    gpiosetd

PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/scripts/switch_boot.sh:$(TARGET_COPY_OUT_SYSTEM)/xbin/switch_boot

PRODUCT_SOONG_NAMESPACES += $(DEVICE_PATH)
PRODUCT_SOONG_NAMESPACES += external/mesa3d
PRODUCT_SOONG_NAMESPACES += packages/apps/Bluetooth

# recovery
# enable when building recoveryimage
PRODUCT_COPY_FILES += $(call find-copy-subdir-files,*,$(LOCAL_PATH)/recovery/root,recovery/root)
PRODUCT_COPY_FILES += $(DEVICE_PATH)/recovery/recovery.fstab:recovery/root/system/etc/recovery.fstab

#PRODUCT_COPY_FILES += \
    $(DEVICE_PATH)/scripts/switch_boot_recovery.sh:recovery/root/sbin/switch_boot_recovery \
    $(DEVICE_PATH)/scripts/switch_boot_rom.zip:recovery/root/switch_boot_rom.zip

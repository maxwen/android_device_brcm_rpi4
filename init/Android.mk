ifeq ($(TARGET_INIT_VENDOR_LIB),libinit_rpi4)

LOCAL_PATH := $(call my-dir)
LIBINIT_MSM_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE_TAGS := optional
LOCAL_C_INCLUDES := \
    system/core/base/include \
    system/core/init
LOCAL_CFLAGS := -Wall -DANDROID_TARGET=\"$(TARGET_BOARD_PLATFORM)\"
LOCAL_SRC_FILES := init_rpi4.cpp
LOCAL_MODULE := libinit_rpi4
include $(BUILD_STATIC_LIBRARY)

endif
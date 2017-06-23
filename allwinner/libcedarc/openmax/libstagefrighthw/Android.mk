
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

MOD_TOP=$(LOCAL_PATH)/../..
include $(MOD_TOP)/config.mk

LOCAL_SRC_FILES := \
    AwOMXPlugin.cpp                      \

LOCAL_CFLAGS += $(PV_CFLAGS_MINUS_VISIBILITY)
LOCAL_CFLAGS += $(AW_OMX_EXT_CFLAGS)
LOCAL_MODULE_TAGS := optional


LOCAL_C_INCLUDES :=    \
        $(MOD_TOP)/ \
        $(MOD_TOP)/include \
        $(MOD_TOP)/base/include \
        $(TOP)/frameworks/native/include/media/hardware \
        $(TOP)/frameworks/native/include/media/openmax \

LOCAL_SHARED_LIBRARIES :=       \
        libbinder               \
        libutils                \
        libcutils               \
        libdl                   \
        libui                   \

LOCAL_MODULE := libstagefrighthw

include $(BUILD_SHARED_LIBRARY)

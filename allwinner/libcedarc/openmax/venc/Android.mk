LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

MOD_TOP=$(LOCAL_PATH)/../..

include $(MOD_TOP)/config.mk

LOCAL_CFLAGS += $(AW_OMX_EXT_CFLAGS)
LOCAL_CFLAGS += -D__OS_ANDROID
TARGET_GLOBAL_CFLAGS += -DTARGET_BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)
LOCAL_MODULE_TAGS := optional

ifeq ($(TARGET_ARCH_VARIANT), armv7-a-neon)
LOCAL_SRC_FILES:= neon_rgb2yuv.s  omx_venc.cpp omx_tsem.c
else
LOCAL_SRC_FILES:= omx_venc.cpp omx_tsem.c
endif


####################### donot support neon in android N ###########
os_version = $(shell echo $(PLATFORM_VERSION) | cut -c 1)
$(warning $(os_version))
ifeq ($(os_version), 7)
LOCAL_SRC_FILES:= omx_venc.cpp omx_tsem.c
endif


###########################################################

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../omxcore/inc/ \
	$(TOP)/frameworks/native/include/     \
	$(TOP)/frameworks/native/include/media/hardware \
	$(TOP)/frameworks/native/include/media/openmax \
	$(TOP)/frameworks/av/media/libcedarc/include \
	$(CEDARC_PATH)/include \
	$(MOD_TOP)/memory/include \
	$(MOD_TOP)/include \
	$(MOD_TOP)/base/include \
	$(MOD_TOP)/ \

LOCAL_C_INCLUDES  += $(TOP)/hardware/libhardware/include/hardware/

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libui \
	libion \
	libVE \
	libMemAdapter \
	libvencoder \
        libcdc_base


LOCAL_MODULE:= libOmxVenc

include $(BUILD_SHARED_LIBRARY)

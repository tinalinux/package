LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

MOD_TOP=$(LOCAL_PATH)/../..
include $(MOD_TOP)/config.mk

LOCAL_CFLAGS += $(AW_OMX_EXT_CFLAGS)
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= transform_color_format.c

ifeq ($(NEW_DISPLAY), yes)
    LOCAL_SRC_FILES += omx_vdec_newDisplay.cpp
else
    LOCAL_SRC_FILES += omx_vdec.cpp
endif

TARGET_GLOBAL_CFLAGS += -DTARGET_BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../omxcore/inc/ \
	$(TOP)/frameworks/native/include/     \
	$(TOP)/frameworks/native/include/media/hardware \
	$(TOP)/frameworks/av/media/libcedarc/include \
	$(TOP)/hardware/libhardware/include/hardware/ \
	$(CEDARC_PATH)/include \
	$(MOD_TOP)/memory/include \
	$(MOD_TOP)/include \
	$(MOD_TOP)/base/include \
	$(MOD_TOP)/ve/include \
	$(MOD_TOP)/

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libui       \
	libdl \
	libVE \
	libvdecoder \
	libvideoengine \
	libMemAdapter \
	libcdc_base \
	libion        \


#libvdecoder
LOCAL_MODULE:= libOmxVdec

include $(BUILD_SHARED_LIBRARY)

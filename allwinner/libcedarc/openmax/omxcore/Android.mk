LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

MOD_TOP=$(LOCAL_PATH)/../..
include $(MOD_TOP)/config.mk

LOCAL_CFLAGS += $(AW_OMX_EXT_CFLAGS)
#LOCAL_CFLAGS += -DLOG_NDEBUG=0
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= src/omx_core_cmp.cpp src/aw_omx_core.c src/aw_registry_table.c

LOCAL_C_INCLUDES := \
    $(MOD_TOP)/ \
    $(MOD_TOP)/include \
    $(MOD_TOP)/base/include \
	$(LOCAL_PATH)/inc/ \

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libbinder \
	libdl

LOCAL_MODULE:= libOmxCore

include $(BUILD_SHARED_LIBRARY)

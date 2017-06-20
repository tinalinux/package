LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

SCLIB_TOP=${LOCAL_PATH}/..
include ${SCLIB_TOP}/config.mk

current_path := $(LOCAL_PATH)

libvdecoder_src_common   :=  fbm.c
libvdecoder_src_common   +=  pixel_format.c
libvdecoder_src_common   +=  sbm.c
libvdecoder_src_common   +=  vdecoder.c

libvdecoder_inc_common :=	$(current_path) \
				$(SCLIB_TOP)/ve/include \
		                    $(SCLIB_TOP)/include \
		                    $(SCLIB_TOP)/base/include \
		                    $(LOCAL_PATH)/include \
		                    $(LOCAL_PATH) \
		                    $(LOCAL_PATH)/videoengine/ \

LOCAL_SRC_FILES := $(libvdecoder_src_common)
LOCAL_C_INCLUDES := $(libvdecoder_inc_common)
LOCAL_CFLAGS :=
LOCAL_LDFLAGS :=


LOCAL_MODULE_TAGS := optional

## add libaw* for eng/user rebuild
LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libui       \
	libdl       \
	libVE       \
	libvideoengine


LOCAL_MODULE := libvdecoder

include $(BUILD_SHARED_LIBRARY)

include $(call all-makefiles-under,$(LOCAL_PATH))

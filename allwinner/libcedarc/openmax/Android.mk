#         OpenMax video decoder of AW's implementation
# compile 7 part:
# (1)libstagefrighthw, adopt for libstagefright module
# (2)omxcore, omx framework to help to generate omx_component
# (3)vdec, the implementation of omx_component for video decoder
# (4)vdecoder, video decode lib of AWself. It's not open for customer.
# (5)venc, the implementation of omx_component for video encoder
# (6)vencode, video encode lib of AWself. It's not open for customer.
# (7)request ve source.
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(call all-makefiles-under,$(LOCAL_PATH))

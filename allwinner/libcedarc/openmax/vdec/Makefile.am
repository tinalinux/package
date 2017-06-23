
include $(top_srcdir)/Makefile.inc

pkglib_LTLIBRARIES = libcdc_omxil_vdec.la

libcdc_omxil_vdec_la_SOURCES = \
						  omx_vdec.cpp \
						  transform_color_format.c

#vdec/omx_vdec_newDisplay.cpp ## not suport new display (bz)

libcdc_omxil_vdec_la_CFLAGS = $(CFLAGS_CDCG)
libcdc_omxil_vdec_la_CPPFLAGS = $(CPPFLAGS_CDCG)
LOCAL_INCLUDE = -I./include \
				-I$(top_srcdir)/include \
				-I$(top_srcdir)/openmax/omxcore/inc \
				-I$(top_srcdir)/base/include \
				-I$(top_srcdir)/ve/include

libcdc_omxil_vdec_la_CFLAGS += $(LOCAL_INCLUDE)
libcdc_omxil_vdec_la_CPPFLAGS += $(LOCAL_INCLUDE)

libcdc_omxil_vdec_la_LDFLAGS = $(LDFLAGS_CDCG)

libcdc_omxil_vdec_la_LIBADD = $(top_srcdir)/base/libcdc_base.la \
						$(top_srcdir)/vdecoder/libcdc_vdecoder.la \
						$(top_srcdir)/memory/libcdc_memory.la
LOCAL_LIB = -ldl -lpthread -lcdc_ve
libcdc_omxil_vdec_la_LDFLAGS += $(LOCAL_LIB)

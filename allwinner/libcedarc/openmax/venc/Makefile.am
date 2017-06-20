
include $(top_srcdir)/Makefile.inc

pkglib_LTLIBRARIES = libcdc_omxil_venc.la

libcdc_omxil_venc_la_SOURCES = \
						  omx_venc.cpp \
						  omx_tsem.c

#vdec/omx_vdec_newDisplay.cpp ## not suport new display (bz)

libcdc_omxil_venc_la_CFLAGS = $(CFLAGS_CDCG)
libcdc_omxil_venc_la_CPPFLAGS = $(CPPFLAGS_CDCG)
LOCAL_INCLUDE = -I./include \
				-I$(top_srcdir)/include \
				-I$(top_srcdir)/openmax/omxcore/inc \
				-I$(top_srcdir)/base/include \
				-I$(top_srcdir)/ve/include

libcdc_omxil_venc_la_CFLAGS += $(LOCAL_INCLUDE)
libcdc_omxil_venc_la_CPPFLAGS += $(LOCAL_INCLUDE)

libcdc_omxil_venc_la_LDFLAGS = $(LDFLAGS_CDCG)

libcdc_omxil_venc_la_LIBADD = $(top_srcdir)/base/libcdc_base.la \
						$(top_srcdir)/memory/libcdc_memory.la

LOCAL_LIB = -ldl -lpthread -lcdc_vencoder -lcdc_ve
libcdc_omxil_venc_la_LDFLAGS += $(LOCAL_LIB)

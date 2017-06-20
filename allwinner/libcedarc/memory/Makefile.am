
include $(top_srcdir)/Makefile.inc

lib_LTLIBRARIES = libcdc_memory.la

libcdc_memory_la_SOURCES =  ./memoryAdapter.c \
							./ionMemory/ionAlloc.c

#			./secureMemory/secureAlloc.c
#			./sunxiMemory/sunxiAlloc.c

libcdc_memory_la_CFLAGS = $(CFLAGS_CDCG)
LOCAL_INCLUDE = -I./include \
				-I$(top_srcdir)/include \
				-I$(top_srcdir)/base/include \
				-I$(top_srcdir)/ve/include

libcdc_memory_la_CFLAGS += $(LOCAL_INCLUDE)


libcdc_memory_la_LDFLAGS = $(LDFLAGS_CDCG)

libcdc_memory_la_LIBADD = $(top_srcdir)/base/libcdc_base.la
#						$(top_srcdir)/ve/libcdc_ve.la

#LOCAL_LIB = -lpthread
LOCAL_LIB = -lcdc_ve

libcdc_memory_la_LDFLAGS += $(LOCAL_LIB)

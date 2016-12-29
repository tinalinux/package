TARGET := dragonboard
INCLUDES += -I include
LIBS:= -lminigui_ths -lpng -ljpeg -ldragon
local_CFLAGS := -D_GNU_SOURCE
local_CFLAGS += -DPACKAGE_NAME=\"\" -DPACKAGE_TARNAME=\"\" -DPACKAGE_VERSION=\"\" -DPACKAGE_STRING=\"\" -DPACKAGE_BUGREPORT=\"\" -DPACKAGE_URL=\"\" -DPACKAGE=\"mg-samples\" -DVERSION=\"3.0.12\" -DSTDC_HEADERS=1 -DHAVE_SYS_WAIT_H=1 -DTIME_WITH_SYS_TIME=1 -DHAVE_SYS_TYPES_H=1 -DHAVE_SYS_STAT_H=1 -DHAVE_STDLIB_H=1 -DHAVE_STRING_H=1 -DHAVE_MEMORY_H=1 -DHAVE_STRINGS_H=1 -DHAVE_INTTYPES_H=1 -DHAVE_STDINT_H=1 -DHAVE_UNISTD_H=1 -DHAVE_SYS_TIME_H=1 -DHAVE_UNISTD_H=1 -DHAVE_MINIGUI_COMMON_H=1 -DHAVE_DECL___NOUNIX__=0 -DHAVE_DECL__MGRM_PROCESSES=1 -D_LANG_EN=1
local_CFLAGS += -I$(STAGING_DIR)/usr/include/allwinner -I$(STAGING_DIR)/usr/include/allwinner/include

common_src_files := \
	dragonview.cpp \
	cameraview.cpp \
	dragonboard.cpp

OBJS = $(patsubst %.cpp, %.cpp.o, $(common_src_files))

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) $(LIBS) -o $@ $^

$(OBJS):%.cpp.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(local_CFLAGS) -c $^ -o $@

all: $(TARGET) install

install:
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	mkdir -p $(INSTALL_PREFIX)/etc
	cp $(TARGET) $(INSTALL_PREFIX)/usr/bin

clean:
	-rm $(OBJS) $(TARGET)

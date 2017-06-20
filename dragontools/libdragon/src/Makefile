TARGET := libdragon.so
common_src_files := \
	script.cpp \
	script_parser.cpp \
	core.cpp

local_CFLAGS := -D_GNU_SOURCE -Werror

OBJS = $(patsubst %.cpp, %.cpp.o, $(common_src_files))

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) $(LIBS) -o $@ $^ -fPIC -shared

$(OBJS):%.cpp.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(local_CFLAGS) -c $^ -o $@ -fPIC -shared

all: $(TARGET) install

install:
	mkdir -p $(INSTALL_PREFIX)/usr/lib
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	mkdir -p $(INSTALL_PREFIX)/usr/include/libdragon
	cp $(TARGET) $(INSTALL_PREFIX)/usr/lib
	cp *.sh $(INSTALL_PREFIX)/usr/bin
	cp *.h $(INSTALL_PREFIX)/usr/include/libdragon

clean:
	-rm $(OBJS) $(TARGET)

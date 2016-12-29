TARGET := libdragonst.so
LIBS:=

common_src_files := \
	AwsDevPlugin.cpp \
	awserdev.cpp \
	awsfostream.cpp \
	awshelper.cpp \
	awsostream.cpp \
	GeneralFuncs.cpp \
	GenSerialPortStream.cpp \
	tinystr.cpp \
	tinyxml.cpp \
	tinyxmlerror.cpp \
	tinyxmlparser.cpp \

local_CFLAGS := -D_GNU_SOURCE -Werror

OBJS = $(patsubst %.cpp, %.cpp.o, $(common_src_files))

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) $(LIBS) -o $@ $^ -fPIC -shared

$(OBJS):%.cpp.o:%.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -c $^ -o $@ -fPIC -shared

all: $(TARGET) install

clean:
	-rm $(OBJS) $(TARGET)
install:
	mkdir -p $(INSTALL_PREFIX)/usr/lib
	mkdir -p $(INSTALL_PREFIX)/usr/include
	cp $(TARGET) $(INSTALL_PREFIX)/usr/lib
	cp *.h $(INSTALL_PREFIX)/usr/include

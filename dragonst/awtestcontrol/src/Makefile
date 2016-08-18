TARGET := awtestcontrol
LIBS:= -lcutils

common_src_files := \
	awsfostream.cpp \
	awshelper.cpp \
	awsostream.cpp \
	awstcrl.cpp \
	GeneralFuncs.cpp \
	GenSerialPortStream.cpp \
	main.cpp \
	tinystr.cpp \
	tinyxml.cpp \
	tinyxmlerror.cpp \
	tinyxmlparser.cpp

local_CFLAGS := -D_GNU_SOURCE -Werror

OBJS = $(patsubst %.cpp, %.cpp.o, $(common_src_files))

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) $(LIBS) -o $@ $^

$(OBJS):%.cpp.o:%.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -c $^ -o $@

all: $(TARGET) install

clean:
	-rm $(OBJS) $(TARGET)
install:
	echo "bin out dir"
	echo $(INSTALL_PREFIX)/bin
	mkdir -p $(INSTALL_PREFIX)/bin
	cp $(TARGET) $(INSTALL_PREFIX)/bin

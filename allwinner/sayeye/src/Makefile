TARGET := sayeye
LIBS:= -lsysutils -lcutils -lutils

common_src_files := \
       SayeyeManager.cpp \
       CommandListener.cpp \
       SayeyeCommand.cpp \
       ResponseCode.cpp \
       SayeyeUtil.cpp \
       ScenseControl.cpp \
       Normal.cpp \
       Home.cpp \
       BootComplete.cpp \
       Video.cpp \
       Music.cpp \
       Rotate.cpp \
       Monitor.cpp \
       BenchMark.cpp \
       sayeye.cpp

local_CFLAGS := -I./include -DANDROID_SMP=1 -DHAVE_SYS_SOCKET_H=1 -D_GNU_SOURCE -Werror

OBJS = $(patsubst %.cpp, %.cpp.o, $(common_src_files))

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) $(LIBS) -o $@ $^

$(OBJS):%.cpp.o:%.cpp
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -c $^ -o $@

sdc:
	$(CC) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -lcutils -o $@ sdc.c

all: $(TARGET) sdc install

clean:
	-rm $(OBJS) $(TARGET)
install:
	mkdir -p $(INSTALL_PREFIX)/bin
	cp $(TARGET) $(INSTALL_PREFIX)/bin
	cp sdc $(INSTALL_PREFIX)/bin

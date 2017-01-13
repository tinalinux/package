TARGET := awtest
INCLUDES += -I include
LIBS:= -ldragonst -ldragon

common_src_files := \
	main.cpp

local_CFLAGS := -D_GNU_SOURCE -Werror

OBJS = $(patsubst %.cpp, %.cpp.o, $(common_src_files))

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) $(LIBS) -o $@ $^

$(OBJS):%.cpp.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(local_CFLAGS) -c $^ -o $@

all: $(TARGET) install

install:
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	cp $(TARGET) $(INSTALL_PREFIX)/usr/bin

clean:
	-rm $(OBJS) $(TARGET)

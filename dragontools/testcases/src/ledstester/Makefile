TARGET := ledtester.sh
LIBS:=

common_src_files := \

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
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	cp $(TARGET) $(INSTALL_PREFIX)/usr/bin

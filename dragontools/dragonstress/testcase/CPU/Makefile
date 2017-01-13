TARGET := StressCPU.sh
INCLUDES += -I ../common
LIBS:=
LOCAL_CFLAGS += -D_GNU_SOURCE

common_src_files := \

OBJS = $(patsubst %.c, %.o, $(common_src_files))

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) $(LIBS) -o $@ $^

$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(LOCAL_CFLAGS)  -c $^ -o $@

all: $(TARGET) install

install:
	cp $(TARGET) $(INSTALL_PREFIX)/usr/bin

clean:
	-rm $(OBJS) $(TARGET)

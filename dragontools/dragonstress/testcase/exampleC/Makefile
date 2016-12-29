TARGET := example
INCLUDE := -I ../common

common_src_files := \
	../common/script.c \
	example.c

LOCAL_CFLAGS += -D_GNU_SOURCE

OBJS = $(patsubst %.c, %.o, $(common_src_files))

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(LOCAL_CFLAGS) -o $@ $^

$(OBJS):%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LDFLAGS) $(LOCAL_CFLAGS) -c $^ -o $@

all: $(TARGET) install

clean:
	-rm $(OBJS) $(TARGET)
install:
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	cp $(TARGET) $(INSTALL_PREFIX)/usr/bin

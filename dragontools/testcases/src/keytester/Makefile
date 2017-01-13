TARGET := keytester
INCLUDE := -I ../common

common_src_files := \
	../common/script.c \
	keytester.c

local_CFLAGS := -D_GNU_SOURCE -Werror

OBJS = $(patsubst %.c, %.c.o, $(common_src_files))

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -o $@ $^

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LDFLAGS) $(local_CFLAGS) -c $^ -o $@

all: $(TARGET) install

clean:
	-rm $(OBJS) $(TARGET)
install:
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	cp $(TARGET) $(INSTALL_PREFIX)/usr/bin

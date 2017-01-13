TARGET := mictester
INCLUDE := -I ../common

common_src_files := \
	../common/script.c \
	pcm_mixer.c \
	tiny_cap_play_sun8iw11p.c

local_CFLAGS := -D_GNU_SOURCE -Werror

OBJS = $(patsubst %.c, %.c.o, $(common_src_files))

$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) -o $@ $^ -lasound

$(OBJS):%.c.o:%.c
	$(CC) $(CFLAGS) $(INCLUDE) $(LDFLAGS) $(local_CFLAGS) -c $^ -o $@

all: $(TARGET) install

clean:
	-rm $(OBJS) $(TARGET)
install:
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	cp $(TARGET) $(INSTALL_PREFIX)/usr/bin
	cp *.sh $(INSTALL_PREFIX)/usr/bin

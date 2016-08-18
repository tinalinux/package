TARGET := mictester
INCLUDE := -I ../../include

common_src_files := \
	../common/script.c \
	pcm_mixer.c \
	sine_detector.c \
	test_record_play.c

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
	cp *.sh $(INSTALL_PREFIX)/usr/bin
	mkdir -p $(INSTALL_PREFIX)/media
	cp *.pcm $(INSTALL_PREFIX)/media
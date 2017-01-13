target	= libsuspend.so

INCLUDES        += -I. -I./include

LOCAL_CFLAGS 	=$(INCLUDES)

SRCS = 	autosuspend.c \
	autosuspend_autosleep.c \
	autosuspend_wakeup_count.c \

ifeq ($(USE_TINA_SUSPEND), y)
SRCS += autosuspend_tinasuspend.c
LOCAL_CFLAGS+=-DUSE_TINA_SUSPEND
else
SRCS += autosuspend_earlysuspend.c
endif

OBJS = $(SRCS:.c=.o)

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp -r ./include/suspend $(CONFIG_PREFIX)/usr/include/

%.o:%.c
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ $<

$(target): $(OBJS)
	$(CC) -fPIC -shared $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install $(target)
clean:
	rm -rf $(target)

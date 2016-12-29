target	= nativepower_daemon

INCLUDES        += -I. -I./include -I$(CONFIG_PREFIX)/include -I$(CONFIG_PREFIX)/usr/include
LIBS            += -lpthread -lpower -lsuspend -luci -ldbus-1

SRCS = main.c \
	power_manager.c \
	dbus_server.c \
	np_uci_config.c \
	np_input.c \
	scenemanager/np_scene_utils.c \
	scenemanager/scene_manager.c

LOCAL_CFLAGS = $(INCLUDES)

ifeq ($(USE_TINA_SUSPEND), y)
LOCAL_CFLAGS += -DUSE_TINA_SUSPEND
endif

ifneq (,$(findstring azalea,$(TARGET_BOARD)))
LOCAL_CFLAGS += -DSUN8IW11P1 $(CFLAGS)
else
LOCAL_CFLAGS += -DSUN50IW1P1 $(CFLAGS)
endif

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./*.h $(CONFIG_PREFIX)/usr/include/
	@cp ./scenemanager/*.h $(CONFIG_PREFIX)/usr/include/

$(target): $(SRCS)
	$(CC) $(CFLAGS) $(LOCAL_CFLAGS) $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install $(target)
clean:
	rm -rf $(target)

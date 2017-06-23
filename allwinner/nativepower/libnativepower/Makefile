target	= libnativepower.so

INCLUDES        += -I. -I./include -I$(CONFIG_PREFIX)/usr/include
LIBS            += -ldbus-1

SRCS = power_manager_client.c \
	dbus_client.c \

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./*.h $(CONFIG_PREFIX)/usr/include/
	@cp ../include/*.h $(CONFIG_PREFIX)/usr/include/

$(target): $(SRCS)
	$(CC) -fPIC -shared $(CFLAGS) $(INCLUDES) $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install $(target)
clean:
	rm -rf $(target)

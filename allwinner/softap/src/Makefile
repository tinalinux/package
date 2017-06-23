target			= libsoftap.so

INCLUDES        += -I. -I./include -I./crypto
LIBS            += -lpthread -ldl -lcrypto

SRCS =wifi.c netd_softap_controller.c command_listener.c

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./include/command_listener.h $(CONFIG_PREFIX)/usr/include
	@cp ./include/response_code.h $(CONFIG_PREFIX)/usr/include

#libsoftap.so
$(target): $(SRCS)
	$(CC) -fPIC -shared $(INCLUDES) $(LDFLAGS) $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install $(target)
clean:
	rm -rf $(target)

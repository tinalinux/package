target	= nativepower_client

INCLUDES        += -I. -I./include -I$(CONFIG_PREFIX)/usr/include
LIBS            += -lnativepower

SRCS = 	main.c

install:


$(target): $(SRCS)
	$(CC) $(CFLAGS) $(INCLUDES) $(LDFLAGS) -L$(CONFIG_PREFIX)/usr/lib $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install $(target)
clean:
	rm -rf $(target)

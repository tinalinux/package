target	= libpower.so

INCLUDES        += -I. -I../include
LIBS            += -lpthread

SRCS = power.c

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./*.h $(CONFIG_PREFIX)/usr/include

$(target): $(SRCS)
	$(CC) -fPIC -shared $(INCLUDES) $(LDFLAGS) $^ -o $@ $(LIBS)
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install $(target)
clean:
	rm -rf $(target)

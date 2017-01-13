target			= libbatinfo.so

INCLUDES        += -I. -I./include
LIBS            := -ldbus-1

SRCS = batinfo.c

OBJS = $(SRCS:.c=.o)

install:$(target)
	@mkdir -p $(CONFIG_PREFIX)/usr/lib
	@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp -r ./include/batinfo $(CONFIG_PREFIX)/usr/include
	@cp $(target) $(CONFIG_PREFIX)/usr/lib

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(target): $(OBJS)
	$(CC) -fPIC -shared $(INCLUDES) $(LDFLAGS) $(LIBS) $^ -o $@

####################################################################
all: install
clean:
	rm -rf $(target) $(OBJS)

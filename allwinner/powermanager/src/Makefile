target	= libpowermanager.so

INCLUDES        += -I. -I./include
LIBS            += -lpthread -ldl

SRCS =PowerManager.c
#OBJS = $(LIB_SRCS:.cpp=.o)

install:
	echo "here is src install"
	-@mkdir -p $(CONFIG_PREFIX)/usr/lib
	-@mkdir -p $(CONFIG_PREFIX)/usr/include
	@cp ./include/PowerManager.h $(CONFIG_PREFIX)/usr/include

#libwifimg.so
$(target): $(SRCS)
	$(CXX) -fPIC -shared $(INCLUDES) $(LDFLAGS) $^ -o $@ $(LIBS)
	echo "here is src install"
	@cp $@ $(CONFIG_PREFIX)/usr/lib

####################################################################
all: install $(target)
clean:
	rm -rf $(target)

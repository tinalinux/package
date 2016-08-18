target	= c_wifimanager

INCLUDES += -I$(CONFIG_PREFIX)/usr/include \
            -I../ \
            -I../mytinaplayer/ \

SRCS = $(target).cpp
OBJS := $(SRCS:.cpp=.o)

all:$(OBJS)

$(OBJS):%.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -g -o $@ $<

clean:
	$(RM) $(target) $(OBJS)

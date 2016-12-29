
INCLUDES += -I$(CONFIG_PREFIX)/usr/include -I../mytinaplayer -I../

SRC := c_bluetooth.cpp
OBJS:=$(SRC:.cpp=.o)

.PHONY: all

all:$(OBJS)

%.o:%.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	@$(RM) $(OBJS)

TARGET = smartlinkd

INCLUDES += -I../ \
			-I../wifimanager/ \
			-I../mytinaplayer/ \

SRCS := $(TARGET).c
OBJS := $(SRCS:%.c=%.o)

all:$(OBJS)

%.o:%.c
	$(CC) -g -c $(CFLAGS) $(CPPFLAG) $(INCLUDES) $< -o $@

clean:
	rm -rf $(TARGET) *.o *.a *~

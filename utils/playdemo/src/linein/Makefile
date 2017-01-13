TARGET := linein

SRCS := $(TARGET).c
OBJS := $(SRCS:.c=.o)
INCLUDES := -I../ -I../mytinaplayer/ -I../bluetooth/ -I../wifimanager/

.PHONY:all clean

all:$(OBJS)

%.o:%.c
	$(CC) -c -g $(CFLAGS) $(INCLUDES) -o $@ $^

clean:
	@rm -rf $(OBJS)

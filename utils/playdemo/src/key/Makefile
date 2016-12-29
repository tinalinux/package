TARGET := key

SRCS := $(TARGET).c
OBJS := $(SRCS:.c=.o)

INCLUDES := -I../ \
            -I../mytinaplayer/ \
            -I../bluetooth/ \
            -I../linein/ \
			-I../powermanager/ \
			-I../localmusic_player/ \

.PHONY:all clean

all:$(OBJS)

%.o:%.c
	$(CC) $(CFLAGS) $(INCLUDES) -c -o $@ $<

clean:
	rm -rf *.o

TARGET  := rwcheck
SRCS     = rwcheck.c crc16.c log.c file_ops.c
OBJS     = $(SRCS:.c=.o)
SUB_LIB += -lpthread

.PHONY:all clean

all:$(OBJS)
	$(CC) $(LDFLAGS) $(SUB_LIB) -o $(TARGET)  $(OBJS)

%.o:%.c
	$(CC) -c $(CFLAGS) -Wall -I. $< -o $@

clean:
	rm -rf $(basename $(SRCS)) $(OBJS) $(TARGET)

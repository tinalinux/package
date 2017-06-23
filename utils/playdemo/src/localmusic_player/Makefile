TARGET = localmusic_player
SRC = $(TARGET).c
OBJ = $(SRC:.c=.o)
INCLUDES = -I../ \
		   -I../mytinaplayer/

.PHONY: all clean

all:$(OBJ)

%.o:%.c
	$(CC) -c $(CFLAGS) $(CPPFLAGS) $(INCLUDES) -o $@ $<

clean:
	rm -rf *.o

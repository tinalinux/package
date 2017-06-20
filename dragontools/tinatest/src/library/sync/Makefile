TARGET := sync.o

.PHONY: all

all: $(TARGET)

$(TARGET):%.o:%.c
	$(CC) $(CFLAGS) $^ -o $@

TARGET := mcollectd.o

.PHONY: all

all: $(TARGET)

$(TARGET):%.o:%.c
	$(CC) $(CFLAGS) $^ -o $@
	$(CP) *.h $(PREFIX)/include

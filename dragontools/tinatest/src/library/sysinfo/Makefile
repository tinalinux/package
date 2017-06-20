TARGET := sysinfo.o

.PHONY: all

all: $(TARGET)

$(TARGET): $(TARGET:.o=.c)
	$(CC) $(CFLAGS) $^ -o $@
	$(CP) *.h $(PREFIX)/include
	$(CP) ./scripts/* $(PREFIX)/bin

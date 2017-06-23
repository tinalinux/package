TARGET := mjson.o
SH_API := mjson_fetch

.PHONY: all

all: $(TARGET)

$(TARGET):%.o:%.c
	$(CC) $(CFLAGS) $^ -o $@
	cp *.h $(PREFIX)/include

$(SH_API): %:%.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(PREFIX)/bin/$@

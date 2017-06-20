.PHONY: all

TARGET := tinatest
SRCS := \
	$(TARGET:%=%.c) \
	task.c

all: $(TARGET)

$(TARGET):
	$(CC) -I$(BUILD_DIR)/src/library/sync $(CFLAGS) $(LDFLAGS) $(SRCS) -o $(PREFIX)/bin/tinatest

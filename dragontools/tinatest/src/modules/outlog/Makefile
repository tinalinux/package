MODULES := $(wildcard outlog_*.c)
MODULES := $(MODULES:%.c=%.o)

.PHONY: all

all: $(MODULES)

%.so: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(PREFIX)/lib/tt-module/$@

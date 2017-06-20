MODULES := $(wildcard mcollectd_*.c)
MODULES := $(MODULES:%.c=%.o)

.PHONY: all

all: $(MODULE)

%.so: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $(PREFIX)/lib/tt-module/$@

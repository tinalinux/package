dirs := \
	exampleC \
	exampleSH \
	CPU \
	IO \
	DDR \
	GPU

#LOCAL_CFLAGS=-DDEBUG

subdirs:
	$(foreach dir, $(dirs), $(MAKE) -C $(dir) all "LOCAL_CFLAGS=$(LOCAL_CFLAGS)" || exit "$$?";)

install:

all: subdirs install

clean:
	echo "clean"

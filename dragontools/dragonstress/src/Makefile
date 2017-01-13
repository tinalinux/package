TARGET := dragonstress
INCLUDES += -I include
#LIBS:= -lminigui_ths -lpng -ljpeg -ldragon
LIBS:= -ldragon -lpthread
local_CFLAGS := -D_GNU_SOURCE -DBOARD=\"$(BOARD)\"

common_src_files := \
	dragonstress.c \
	ds_html.c

OBJS = $(patsubst %.c, %.o, $(common_src_files))

$(TARGET):$(OBJS)
	$(CXX) $(CFLAGS) $(LDFLAGS) $(local_CFLAGS) $(LIBS) -o $@ $^ ./libhtml.a

$(OBJS):%.o:%.c
	$(CXX) $(CFLAGS) $(INCLUDES) $(LDFLAGS) $(local_CFLAGS)  -c $^ -o $@

all: $(TARGET) install

install:
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	cp $(TARGET) $(INSTALL_PREFIX)/usr/bin

clean:
	-rm $(OBJS) $(TARGET)

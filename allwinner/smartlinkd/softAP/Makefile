CC ?= g++
TARGET = smartlink_softAP
INCLUDES += -I. \
			-I./include \
#			-I../../softap/src/include \

SRCS := \
	softAP_demo.c \
	src/wifi_connect_ap.cpp \
	src/softap_operat.c

SUB_LIB := \
	-lpthread \
	-lsoftap \
	-lwifimg

$(TARGET):
	$(CC) $(LDFLAGS) $(INCLUDES) $(SRCS) -o $@ $(SUB_LIB)

all:$(TARGET) install

clean:
	rm -rf $(TARGET) *.o *.a *~

install:
	mkdir -p $(CONFIG_PREFIX)/use/bin
	cp $(TARGET) $(CONFIG_PREFIX)/usr/bin

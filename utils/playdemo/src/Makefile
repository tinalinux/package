TARGET      := playdemo

SRCS    = main.c
OBJS    = $(SRCS:.c=.o) \
		  ./key/key.o \
		  ./smartlinkd/smartlinkd.o \
		  ./wifimanager/c_wifimanager.o \
		  ./linein/linein.o \
		  ./mytinaplayer/c_tinaplayer.o \
		  ./powermanager/c_powermanager.o \
		  ./bluetooth/c_bluetooth.o \
		  ./localmusic_player/localmusic_player.o \

#for smartlinkd
SUB_LIB += \
	-lpthread \
	-lsmartlinkd_client
#for wifimanager
SUB_LIB += \
	-lwifimg
#for bluetooth
SUB_LIB += \
	-lbtapp \
	-lbsa \
#for tinaplayer
SUB_LIB += -ltinaplayer -lxplayer -lcdc_vdecoder -lcdc_adecoder -lcdc_sdecoder -lcdc_base -lcdc_ve -lcdc_memory \
		   -lcdx_parser  -lplayer -lcdx_stream -lcdx_base  -lpostprocess \
			-law_plugin  -lpthread -ldl -lstdc++ -lrt -lm -lc -lz -lasound
SUB_LIB += -lpowermanager -ldl

.PHONY:all clean

all:$(SRCS:.c=.o)
	$(Build/Key)
	$(Build/Smartlinkd)
	$(Build/Wifimanager)
	$(Build/Bluetooth)
	$(Build/Linein)
	$(Build/c_tinaplayer)
	$(Build/c_powermanager)
	$(Build/localmusic_player)
	$(CC) $(LDFLAGS) $(SUB_LIB) -o $(TARGET)  $(OBJS)

%.o:%.c
	$(CC) -c $(CFLAGS) -Wall $(foreach tmp,$(dir $(OBJS)),-I$(tmp)) $< -o $@

define Build/localmusic_player
	#complie localmusic_player
	$(MAKE) -C ./localmusic_player/ \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(CFLAGS) -Wall" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/c_powermanager
	#complie c_powermanager
	$(MAKE) -C ./powermanager/ \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(CFLAGS) -Wall" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Key
	#complie Key
	$(MAKE) -C ./key/ \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(CFLAGS) -Wall" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/c_tinaplayer
	#complie c_tinaplayer
	$(MAKE) -C ./mytinaplayer/ \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(CFLAGS) -Wall" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Linein
	#complie linein's demo
	$(MAKE) -C ./linein/ \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(CC)" \
		CFLAGS="$(CFLAGS) -Wall" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Smartlinkd
	#complie smartlinkd's demo
	$(MAKE) -C ./smartlinkd/ \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(CC)" \
		CFLAGS="$(CFLAGS) -Wall" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Wifimanager
	#wifimanager_demo
	$(MAKE) -C ./wifimanager/ \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Bluetooth
	#compile bt_demo
	$(MAKE) -C ./bluetooth/ \
		-f Makefile \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

clean:
	rm -rf $(basename $(SRCS)) $(OBJS) $(TARGET)

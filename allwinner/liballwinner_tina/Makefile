include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=liballwinner
PKG_VERSION:=1
PKG_RELEASE:=1

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),astar octopus))
PKG_BUILD_DIR := $(COMPILE_DIR)/liballwinner_old
else ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),azalea tulip sitar cello banjo))
PKG_BUILD_DIR := $(COMPILE_DIR)/liballwinner_new
endif

include $(BUILD_DIR)/package.mk

TINA_CHIP_TYPE = r16
TINA_PRODUCT_TYPE = tvbox

TINA_CHIP_TYPE = G102
KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux-muslgnueabi
CEDARC_LIBS_PATH = libcedarc/lib/arm-openwrt-linux-muslgnueabi
CEDARX_LIBS_PATH = libcedarx/external/lib32/arm-openwrt-linux-muslgnueabi

VIDEO_DECODE_LIB := libcdc_vd_avs.so \
                    libcdc_vd_h264.so \
                    libcdc_vd_h265.so \
                    libcdc_vd_mjpegplus.so \
                    libcdc_vd_mjpeg.so \
                    libcdc_vd_mpeg2.so \
                    libcdc_vd_mpeg4base.so \
                    libcdc_vd_mpeg4h263.so \
                    libcdc_vd_mpeg4normal.so \
                    libcdc_vd_mpeg4vp6.so \
                    libcdc_vd_vp8.so \
                    libcdc_vd_wmv3.so \
                    libcdc_vd_mpeg4dx.so
SYSTEM_LIB := libasound.so \
              libcrypto.so \
              libssl.so \
              libxml2.so \
              libz.so

ifeq ($(TARGET_BOARD_PLATFORM),astar)
	TINA_CHIP_TYPE = r16
endif

ifeq ($(TARGET_BOARD_PLATFORM),octopus)
	TINA_CHIP_TYPE = r58
endif

ifeq ($(TARGET_BOARD_PLATFORM),azalea)
	TINA_CHIP_TYPE = t3
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
	CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux-muslgnueabi
	CEDARC_LIBS_PATH = libcedarc/lib/arm-openwrt-linux-muslgnueabi
	CEDARX_LIBS_PATH = libcedarx/external/lib32/arm-openwrt-linux-muslgnueabi
endif

ifeq ($(TARGET_BOARD_PLATFORM),tulip)
	TINA_CHIP_TYPE = r18
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
	CEDARC_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux
	CEDARC_LIBS_PATH = libcedarc/lib/arm-aarch64-openwrt-linux
	CEDARX_LIBS_PATH = libcedarx/external/lib64/arm-aarch64-openwrt-linux
endif

ifeq ($(TARGET_BOARD_PLATFORM),sitar)
	TINA_CHIP_TYPE = c600
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
	CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux-muslgnueabi
	CEDARC_LIBS_PATH = libcedarc/lib/arm926-muslgnueabi
	CEDARX_LIBS_PATH = libcedarx/external/lib32/arm926-muslgnueabi
endif

ifeq ($(TARGET_BOARD_PLATFORM),cello)
	TINA_CHIP_TYPE = G102
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
	CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux-muslgnueabi
	CEDARC_LIBS_PATH = libcedarc/lib/arm-openwrt-linux-muslgnueabi
	CEDARX_LIBS_PATH = libcedarx/external/lib32/arm-openwrt-linux-muslgnueabi
endif

ifeq ($(TARGET_BOARD_PLATFORM),banjo)
	TINA_CHIP_TYPE = MR100
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_4
	CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux-muslgnueabi
	CEDARC_LIBS_PATH = libcedarc/lib/arm-openwrt-linux-muslgnueabi
	CEDARX_LIBS_PATH = libcedarx/external/lib32/arm-openwrt-linux-muslgnueabi
endif

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),astar octopus))

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=CedarX2.6  for allwinner
  DEPENDS:=+libpthread \
	   +libopenssl \
	   +libstdcpp \
	   +zlib \
	   +alsa-lib
endef

define Package/$(PKG_NAME)/config
endef

define Package/$(PKG_NAME)/Default
  TITLE:=liballwinner for all
  URL:=http://www.allwinner.com/
endef

define Package/$(PKG_NAME)/description
	CedarX2.6
endef

define Package/jpegdecodedemo
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=CedarX2.6 tina jpeg decode utils for allwinner
  DEPENDS:=+liballwinner
endef

define Package/jpegdecodedemo/description
	CedarX2.6 tina jpeg decode
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ../liballwinner/LIBRARY $(PKG_BUILD_DIR)/
	$(CP) -r ../liballwinner/awrecorder $(PKG_BUILD_DIR)/
	$(CP) -r ../liballwinner/xplayer $(PKG_BUILD_DIR)/
	$(CP) -r ../liballwinner/xmetadataretriever $(PKG_BUILD_DIR)/
	$(CP) -r ../liballwinner/Makefile.inc $(PKG_BUILD_DIR)/
	$(CP) -r ./tinaplayer_old $(PKG_BUILD_DIR)/tinaplayer
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
	#cd $(PKG_BUILD_DIR)/; rm -rf ./LIBRARY/EXTERNAL/include/alsa/ ./LIBRARY/EXTERNAL/include/libxml/ ./LIBRARY/EXTERNAL/include/openssl/ ./LIBRARY/EXTERNAL/include/zlib/
	#cd $(PKG_BUILD_DIR)/LIBRARY/EXTERNAL/lib32/muslgnueabi/; rm -rf $(SYSTEM_LIB)
endef

define Build/Configure
	$(info *****************target_board_platform = $(TARGET_BOARD_PLATFORM)**************)
	$(info *****************chip type = $(TINA_CHIP_TYPE)**************)
	cd $(PKG_BUILD_DIR); ./LIBRARY/configure --tool-chain=muslgnueabi --cedarx-path=$(PKG_BUILD_DIR) --chip=$(TINA_CHIP_TYPE) --product=$(TINA_PRODUCT_TYPE) --ssl-enable=yes --live555-enable=yes --mms=yes

endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/LIBRARY \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		PREFIX="$(PKG_INSTALL_DIR)" \
		all \
		install

	$(MAKE) -C $(PKG_BUILD_DIR)/tinaplayer \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"

	$(MAKE) -C $(PKG_BUILD_DIR)/demo/jpegdecodedemo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"

# when compile fisished, the targets should be copy to $(PKG_INSTALL_DIR)
endef

define Build/InstallDev
	mkdir -p $(PKG_INSTALL_DIR)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner
	############################################################

	## copy libraries
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/install/lib/*.so                           $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/install/external/*.so                      $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/tinaplayer/libtinaplayer.so                        $(PKG_INSTALL_DIR)/usr/lib/
	cd $(PKG_INSTALL_DIR)/usr/lib/; rm -rf $(SYSTEM_LIB)
	#################################################################

	#####################################################################

	## copy header files.
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/cdx_config.h                                $(PKG_INSTALL_DIR)/usr/include/allwinner/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/log.h                                   $(PKG_INSTALL_DIR)/usr/include/allwinner/
	$(CP) $(PKG_BUILD_DIR)/LIBRARY/version.h                               $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## DEMUX
	-cp -r $(PKG_BUILD_DIR)/LIBRARY/DEMUX/BASE/include                   $(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/LIBRARY/DEMUX/STREAM/include                 $(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/LIBRARY/DEMUX/PARSER/include                 $(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/LIBRARY/DEMUX/MUXER/include                  $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## CODEC
	-cp $(PKG_BUILD_DIR)/LIBRARY/libcedarc/include/*.h                   $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## PLAYER
	-cp $(PKG_BUILD_DIR)/LIBRARY/PLAYER/include/soundControl.h                         $(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp $(PKG_BUILD_DIR)/LIBRARY/PLAYER/include/layerControl.h                         $(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp $(PKG_BUILD_DIR)/LIBRARY/PLAYER/include/player.h                         $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## xplayer
	-cp $(PKG_BUILD_DIR)/xplayer/awplayer.h                                      $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## awrecorder
	-cp $(PKG_BUILD_DIR)/awrecorder/awencoder.h                                  $(PKG_INSTALL_DIR)/usr/include/allwinner/
	## tinaplayer
	-cp $(PKG_BUILD_DIR)/tinaplayer/tinaplayer.h                                 $(PKG_INSTALL_DIR)/usr/include/allwinner/
	#########################################################################################
	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/usr
# the targets should be copy to $(1),for global symbol for other package
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	############################################################
	# copy libraries
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/install/lib/*.so                           $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/LIBRARY/install/external/*.so                      $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinaplayer/libtinaplayer.so                        $(1)/usr/lib/

	cd $(1)/usr/lib/; rm -rf $(SYSTEM_LIB)

endef

define Package/jpegdecodedemo/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/demo/jpegdecodedemo/jpegdecodedemo				 $(1)/usr/bin/
endef


else ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),azalea tulip sitar cello banjo))

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Allwinner
	TITLE:=CedarX2.7  for allwinner
	DEPENDS:=+libpthread \
	   +libopenssl \
	   +libstdcpp \
	   +zlib \
	   +alsa-lib

endef

define Package/$(PKG_NAME)/config
endef

define Package/$(PKG_NAME)/Default
  TITLE:=liballwinner for all
  URL:=http://www.allwinner.com/
endef

define Package/$(PKG_NAME)/description
	CedarX2.7
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ../libcedarc $(PKG_BUILD_DIR)/
	$(CP) -r ../libcedarx $(PKG_BUILD_DIR)/
	$(CP) -r ./tinaplayer_new $(PKG_BUILD_DIR)/tinaplayer
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
	$(CP) -r $(TARGET_BOARD_PLATFORM)_linux_cedarx.conf $(PKG_BUILD_DIR)/
endef

define Build/Configure
	$(info *****************target_board_platform = $(TARGET_BOARD_PLATFORM)**************)
	$(info *****************chip type = $(TINA_CHIP_TYPE)**************)
	cd $(PKG_BUILD_DIR)/libcedarc; \
	./bootstrap; \
	./configure --prefix=$(PKG_BUILD_DIR)/libcedarc/install --host=$(CEDARC_COMPILE_TOOL_CHAIN) CFLAGS="-D__ENABLE_ZLIB__ -D$(KERNEL_VERSION)" CPPFLAGS="-D__ENABLE_ZLIB__ -D$(KERNEL_VERSION)" LDFLAGS="-L$(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)"
	cd $(PKG_BUILD_DIR)/libcedarx; \
	./bootstrap; \
	./configure --prefix=$(PKG_BUILD_DIR)/libcedarx/install --host=$(CEDARX_COMPILE_TOOL_CHAIN) CFLAGS="-D__ENABLE_ZLIB__" CPPFLAGS="-D__ENABLE_ZLIB__" LDFLAGS="-lcrypto -lz -L$(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH) -L$(PKG_BUILD_DIR)/libcedarc/install/lib -L$(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)"
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/libcedarc \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D__ENABLE_ZLIB__ -D$(KERNEL_VERSION)" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -D__ENABLE_ZLIB__ -D$(KERNEL_VERSION)" \
		LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH) -L$(PKG_BUILD_DIR)/libcedarc/install/lib" \
		install
	$(MAKE) -C $(PKG_BUILD_DIR)/libcedarx \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D__ENABLE_ZLIB__" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -D__ENABLE_ZLIB__" \
		LDFLAGS="$(TARGET_LDFLAGS) -lcrypto -lz -L$(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH) -L$(PKG_BUILD_DIR)/libcedarc/install/lib -L$(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH) -L$(PKG_BUILD_DIR)/libcedarx/install/lib" \
		install
	$(MAKE) -C $(PKG_BUILD_DIR)/tinaplayer \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Build/InstallDev
	mkdir -p $(PKG_INSTALL_DIR)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner
	## copy libraries
	$(CP) $(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)/*.so							$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libcedarc/install/lib/*.so								$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libcedarc/install/lib/full-package-name/*.so				$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/*.so							$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libcedarx/install/lib/*.so								$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/tinaplayer/libtinaplayer.so								$(PKG_INSTALL_DIR)/usr/lib/
	cd $(PKG_INSTALL_DIR)/usr/lib/; rm -rf $(SYSTEM_LIB)

	## copy header files.

	## log.h
	$(CP) $(PKG_BUILD_DIR)/libcedarc/base/include/log.h								$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## libcedarc vdecoder/vencoder
	-cp -r $(PKG_BUILD_DIR)/libcedarc/include											$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## libcedarx adecoder/aencoder/sdecoder
	-cp -r $(PKG_BUILD_DIR)/libcedarx/external/include/adecoder/adecoder.h					$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/libcedarx/external/include/aencoder/aencoder.h					$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/libcedarx/external/include/sdecoder/sdecoder.h					$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## libcedarx:libcore
	-cp -r $(PKG_BUILD_DIR)/libcedarx/libcore/base/include/*.h								$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/libcedarx/libcore/muxer/include/*.h								$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/libcedarx/libcore/parser/include/*.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/libcedarx/libcore/stream/include/*.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/libcedarx/libcore/playback/include/*.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## xplayer
	-cp -r $(PKG_BUILD_DIR)/libcedarx/xplayer/include/*.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## tinaplayer
	-cp $(PKG_BUILD_DIR)/tinaplayer/tinaplayer.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## awrecorder
	-cp $(PKG_BUILD_DIR)/libcedarx/awrecorder/awencoder.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/

	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/usr

endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/
	############################################################

	## copy libraries
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)/*.so						$(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libcedarc/install/lib/*.so							 $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libcedarc/install/lib/full-package-name/*.so			$(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/*.so						$(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libcedarx/install/lib/*.so							$(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinaplayer/libtinaplayer.so								$(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libcedarx/install/bin/xplayerdemo						 $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libcedarx/install/bin/recoderdemo						 $(1)/usr/bin/
	$(CP)          $(PKG_BUILD_DIR)/$(TARGET_BOARD_PLATFORM)_linux_cedarx.conf						 $(1)/etc/cedarx.conf
	cd $(1)/usr/lib/; rm -rf $(SYSTEM_LIB)

endef

endif

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),astar octopus azalea tulip sitar cello banjo))
$(eval $(call BuildPackage,$(PKG_NAME)))
endif

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),astar octopus))
$(eval $(call BuildPackage,jpegdecodedemo))
endif

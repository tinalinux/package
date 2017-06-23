include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=libcedarx2.7
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/libcedarx2.7

include $(BUILD_DIR)/package.mk

TINA_CHIP_TYPE = G102

TINA_CHIP_TYPE = G102
KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux-muslgnueabi
CEDARC_LIBS_PATH = libcedarc/lib/arm-openwrt-linux-muslgnueabi
CEDARX_LIBS_PATH = libcedarx/external/lib32/arm-openwrt-linux-muslgnueabi

SYSTEM_LIB := libasound.so \
              libcrypto.so \
              libssl.so \
              libxml2.so \
              libz.so

NOT_SUPPORT_VIDEO_LIBS := libcdc_vd_h265.so

ifeq ($(TARGET_BOARD_PLATFORM),astar)
	TINA_CHIP_TYPE = r16
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_4
	CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux-muslgnueabi
	CEDARC_LIBS_PATH = libcedarc/lib/arm-openwrt-linux-muslgnueabi
	CEDARX_LIBS_PATH = libcedarx/external/lib32/arm-openwrt-linux-muslgnueabi
	NOT_SUPPORT_VIDEO_LIBS += libcdc_vd_avs.so \
							libcdc_vd_mpeg4vp6.so \
							libcdc_vd_mjpegs.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),octopus)
	TINA_CHIP_TYPE = r58
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_4
	CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux-muslgnueabi
	CEDARC_LIBS_PATH = libcedarc/lib/arm-openwrt-linux-muslgnueabi
	CEDARX_LIBS_PATH = libcedarx/external/lib32/arm-openwrt-linux-muslgnueabi
	NOT_SUPPORT_VIDEO_LIBS += libcdc_vd_avs.so \
							libcdc_vd_mpeg4vp6.so \
							libcdc_vd_mjpegs.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),azalea)
	TINA_CHIP_TYPE = r40
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
	CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux-muslgnueabi
	CEDARC_LIBS_PATH = libcedarc/lib/arm-openwrt-linux-muslgnueabi
	CEDARX_LIBS_PATH = libcedarx/external/lib32/arm-openwrt-linux-muslgnueabi
	NOT_SUPPORT_VIDEO_LIBS += libcdc_vd_mjpegs.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),tulip)
	TINA_CHIP_TYPE = r18
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
	CEDARC_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux
	CEDARC_LIBS_PATH = libcedarc/lib/arm-aarch64-openwrt-linux
	CEDARX_LIBS_PATH = libcedarx/external/lib64/arm-aarch64-openwrt-linux
	NOT_SUPPORT_VIDEO_LIBS += libcdc_vd_mpeg4vp6.so \
							 libcdc_vd_mjpeg.so

endif

ifeq ($(TARGET_BOARD_PLATFORM),sitar)
	TINA_CHIP_TYPE = c600
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
	CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux-muslgnueabi
	CEDARC_LIBS_PATH = libcedarc/lib/arm926-muslgnueabi
	CEDARX_LIBS_PATH = libcedarx/external/lib32/arm926-muslgnueabi
	NOT_SUPPORT_VIDEO_LIBS += libcdc_vd_vp8.so \
							 libcdc_vd_mjpegs.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),cello)
	TINA_CHIP_TYPE = G102
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
	CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux-muslgnueabi
	CEDARC_LIBS_PATH = libcedarc/lib/arm-openwrt-linux-muslgnueabi
	CEDARX_LIBS_PATH = libcedarx/external/lib32/arm-openwrt-linux-muslgnueabi
	NOT_SUPPORT_VIDEO_LIBS += libcdc_vd_mpeg2.so \
							libcdc_vd_mpeg4h263.so \
							libcdc_vd_vp8.so \
							libcdc_vd_avs.so \
							libcdc_vd_mpeg4base.so \
							libcdc_vd_mpeg4normal.so \
							libcdc_vd_wmv3.so \
							libcdc_vd_mpeg4h263.so \
							libcdc_vd_mpeg4dx.so \
							libcdc_vd_mpeg4vp6.so \
							libcdc_vd_mjpeg.so \
							libcdc_vd_mjpegs.so \
							libcdc_ve.so \
							libcdc_videoengine.so \
							libcdc_vencoder.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),banjo)
	TINA_CHIP_TYPE = MR100
	KERNEL_VERSION = CONF_KERNEL_VERSION_3_4
	CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux-muslgnueabi
	CEDARC_LIBS_PATH = libcedarc/lib/arm-openwrt-linux-muslgnueabi
	CEDARX_LIBS_PATH = libcedarx/external/lib32/arm-openwrt-linux-muslgnueabi
	NOT_SUPPORT_VIDEO_LIBS += libcdc_vd_mpeg2.so \
							libcdc_vd_mpeg4h263.so \
							libcdc_vd_vp8.so \
							libcdc_vd_avs.so \
							libcdc_vd_mpeg4base.so \
							libcdc_vd_mpeg4normal.so \
							libcdc_vd_wmv3.so \
							libcdc_vd_mpeg4dx.so \
							libcdc_vd_mpeg4vp6.so \
							libcdc_vd_mjpeg.so
endif


ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),astar octopus azalea tulip sitar cello banjo))

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
  TITLE:=libcedarx2.7 for all
  URL:=http://www.allwinner.com/
endef

define Package/$(PKG_NAME)/description
	CedarX2.7
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ../libcedarc $(PKG_BUILD_DIR)/
	$(CP) -r ../libcedarx $(PKG_BUILD_DIR)/
	$(CP) -r ./tplayer $(PKG_BUILD_DIR)/tplayer
	$(CP) -r ./tplayer/configs/$(TARGET_BOARD_PLATFORM)_linux_cedarx.conf $(PKG_BUILD_DIR)/
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

	$(MAKE) -C $(PKG_BUILD_DIR)/tplayer \
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
	$(CP) $(PKG_BUILD_DIR)/tplayer/libtplayer.so									$(PKG_INSTALL_DIR)/usr/lib/
	cd $(PKG_INSTALL_DIR)/usr/lib/; rm -rf $(SYSTEM_LIB)

	## copy header files.

	## log.h
	$(CP) $(PKG_BUILD_DIR)/libcedarc/base/include/log.h								$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## libcedarc vdecoder/vencoder
	-cp -r $(PKG_BUILD_DIR)/libcedarc/include											$(PKG_INSTALL_DIR)/usr/include/allwinner/

	##cdx_config.h
	-cp -r $(PKG_BUILD_DIR)/libcedarx/cdx_config.h										$(PKG_INSTALL_DIR)/usr/include/allwinner/

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

	## tplayer
	-cp $(PKG_BUILD_DIR)/tplayer/tplayer.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/

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
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tplayer/libtplayer.so								$(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libcedarx/install/bin/xplayerdemo						 $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libcedarx/install/bin/recoderdemo						 $(1)/usr/bin/
	$(CP)          $(PKG_BUILD_DIR)/$(TARGET_BOARD_PLATFORM)_linux_cedarx.conf						 $(1)/etc/cedarx.conf
	cd $(1)/usr/lib/; rm -rf $(SYSTEM_LIB) $(NOT_SUPPORT_VIDEO_LIBS)

endef

endif

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),astar octopus azalea tulip sitar cello banjo))
$(eval $(call BuildPackage,$(PKG_NAME)))
endif

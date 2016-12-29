include $(TOPDIR)/rules.mk

PKG_NAME:=dragonboard
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/dragonboard
  SECTION:=test
  CATEGORY:=DragonTools
  SUBMENU:=Dragonboard
  TITLE:=dragonboard for hardware board test
  DEPENDS:= +libminigui-gpl +libpng +libjpeg +libstdcpp +libdragon +liballwinner +liballwinner-base +libcutils
endef

define Package/dragonboard/description
  dragonboard for hardware board test
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/dragonboard/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dragonboard $(1)/usr/bin
	$(CP) dragonboard.init $(1)/etc/init.d/dragonboard
endef

$(eval $(call BuildPackage,dragonboard))

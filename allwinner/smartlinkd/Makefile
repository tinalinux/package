##############################################
# OpenWrt Makefile for smartlinkd program
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to
# specify a basic description of the package,
# where to build our program, where to find
# the source files, and where to install the
# compiled program on the router.
#
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and
# there should be no trailing whitespace in
# lines that are not commented.
#
##############################################
include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=smartlinkd
PKG_VERSION:=0.0.1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:=+libpthread +libstdcpp +liballwinner-base +alsa-lib +wifimanager
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=smartlinkd for Tina
  MENU:=1
endef

define Package/$(PKG_NAME)-demo
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=smartlinkd demo
  DEPENDS:=$(PKG_NAME)
endef

#dhd_priv ,add by Kirin
define Package/$(PKG_NAME)-dhd_priv
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=smartlinkd dhd_priv
  DEPENDS:=$(PKG_NAME)
endef

#smartlink_raw ,add by Kirin
define Package/$(PKG_NAME)-smartlink_raw
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=smartlinkd smartlink_raw
  DEPENDS:=$(PKG_NAME)
endef

define Package/$(PKG_NAME)/description
	wifi configuration for none input platform
endef

define Package/$(PKG_NAME)-demo/description
	client demo
endef

#dhd_priv demo,add by Kirin
define Package/$(PKG_NAME)-dhd_priv/description
	dhd_priv demo
endef

#smartlink_raw demo,add by Kirin
define Package/$(PKG_NAME)-smartlink_raw/description
	smartlink_raw demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
	$(CP) -r ./dhd_priv $(PKG_BUILD_DIR)/
	$(CP) -r ./raw $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile

	#compile libs and service
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		SAVE_DIR="`pwd`" \
		all

	#compile demo
	$(MAKE) -C $(PKG_BUILD_DIR)/demo \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

	#compile smartlink_raw
	$(MAKE) -C $(PKG_BUILD_DIR)/raw \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

	#compile dhd_priv
	$(MAKE) -C $(PKG_BUILD_DIR)/dhd_priv \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include

	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
	$(INSTALL_DATA) src/include/smartlink_util.h $(1)/usr/include
	$(INSTALL_DATA) src/libclient/aw_smartlinkd_connect.h $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/init.d

	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libsmartlinkd_client.so $(1)/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/smartlinkd_setup $(1)/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/smartlinkd_adt $(1)/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/smartlinkd $(1)/bin/

	$(INSTALL_BIN) ./tina/smartlinkd.init $(1)/etc/init.d/smartlinkd
	$(INSTALL_BIN) ./tina/wpa.sh $(1)/etc/wifi

endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/smartlinkd_demo $(1)/usr/bin
endef

#dhd_priv,add by Kirin
define Package/$(PKG_NAME)-dhd_priv/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dhd_priv $(1)/usr/bin
endef

#smartlink_raw,add by Kirin
define Package/$(PKG_NAME)-smartlink_raw/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/smartlink_raw $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
$(eval $(call BuildPackage,$(PKG_NAME)-dhd_priv))
$(eval $(call BuildPackage,$(PKG_NAME)-smartlink_raw))
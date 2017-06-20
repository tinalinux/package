include $(TOPDIR)/rules.mk

PKG_NAME:=softap
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/softap/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=Tina softap manager
  DEPENDS:=+libpthread +libstdcpp +liballwinner-base +libopenssl
endef

define Package/softap
  $(call Package/softap/Default)
  MENU:=1
  DEFAULT:=1
endef

define Package/softap-demo
  $(call Package/softap/Default)
  TITLE:=Tina softap app demo
  DEPENDS := softap +softap
endef

define Package/softap/description
	Tina softap manager
endef

define Package/softap-demo/description
	Tina softap app demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	#libsoftap.so
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

	#softap_test
	$(MAKE) -C $(PKG_BUILD_DIR)/demo/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libsoftap.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/softap/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libsoftap.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DATA) ./src/hostapd.conf $(1)/etc/wifi
#	$(INSTALL_DATA) ./src/wpa_supplicant_overlay.conf $(1)/etc/wifi
#	$(INSTALL_BIN) ./wifi.init $(1)/etc/wifi/wifi
#	$(INSTALL_BIN) ./udhcpc_wlan0.init $(1)/etc/wifi/udhcpc_wlan0
endef

define Package/softap-demo/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/softap_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/softap_up $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/softap_down $(1)/bin
#	$(INSTALL_BIN) ./wifitest.sh $(1)/etc/wifi
endef

$(eval $(call BuildPackage,softap))
$(eval $(call BuildPackage,softap-demo))

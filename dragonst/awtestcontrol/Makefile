include $(TOPDIR)/rules.mk

PKG_NAME:=awtestcontrol
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/awtestcontrol
  SECTION:=utils
  CATEGORY:=DragonST
  TITLE:=awtestcontrol for tina linux dragonboard
  DEPENDS:= +libstdcpp +libcutils
endef

define Package/awtestcontrol/description
	it comes form Android 4.4!!
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
		CXX="$(TARGET_CXX)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all

endef

define Build/InstallDev
	#$(INSTALL_DIR) $(1)/usr/lib
	#$(INSTALL_DIR) $(1)/usr/include
#	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/awtestcontrol/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/bin/
endef

$(eval $(call BuildPackage,awtestcontrol))

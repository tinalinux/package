include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=PowerManager
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/PowerManager/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=Tina power manager
  DEPENDS:=+libpthread  +libstdcpp +liballwinner-base
endef

define Package/PowerManager
  $(call Package/PowerManager/Default)
  MENU:=1
  DEFAULT:=1
endef


define Package/PowerManager-demo
  $(call Package/PowerManager/Default)
  TITLE:=PowerManager demo
  DEPENDS:=PowerManager +libpthread
#  DEPENDS:=PowerManager +libpowermanager
endef


define Package/PowerManager/description
  simple PowerManager for tina
endef



define Package/PowerManager-demo/description
  PowerManager demo for tina
endef


define Build/Prepare
	echo "here is  Prepare"
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
	echo "here is  Configure"
endef

define Build/Compile
	echo "here is  Compile"
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

	echo "Here is InstallDev 64"

	$(MAKE) -C $(PKG_BUILD_DIR)/demo/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" -Wall \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

endef

define Build/InstallDev
	echo "Here is InstallDev"
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libpowermanager.so $(1)/usr/lib

endef

define Package/PowerManager/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libpowermanager.so $(1)/usr/lib


endef

#define Package/PowerManager/InstallDev
#	$(INSTALL_DIR) $(1)/usr/lib/
#endef

define Package/PowerManager-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/PowerManager_demo $(1)/usr/bin

endef
$(eval $(call BuildPackage,PowerManager))
$(eval $(call BuildPackage,PowerManager-demo))

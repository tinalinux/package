include $(TOPDIR)/rules.mk

PKG_NAME:=dragonstress
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/dragonstress
  SECTION:=test
  CATEGORY:=DragonTools
  TITLE:=dragonstress for stress test
  DEPENDS:= +libdragon
endef

define Package/dragonstress/description
  dragonstress for stress test
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src $(PKG_BUILD_DIR)
	$(CP) ./testcase $(PKG_BUILD_DIR)
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
		BOARD=$(BOARD) \
		all
	$(MAKE) -C $(PKG_BUILD_DIR)/testcase \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all

endef

define Package/dragonstress/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) ./files/dragonstress_config.fex $(1)/etc
	$(INSTALL_BIN) ./files/dragonstress.html $(1)/etc
	$(INSTALL_BIN) ./files/script_parser_stress.sh $(1)/usr/bin
	echo PKG_INSTALL_DIR=$(PKG_INSTALL_DIR)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,dragonstress))

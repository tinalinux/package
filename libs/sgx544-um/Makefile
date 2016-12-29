include $(TOPDIR)/rules.mk

# Name and release number of this package
PKG_NAME:=sgx544-um
PKG_VERSION:=1.1.2
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk
 
define Package/sgx544-um
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=SGX544 userspace binary files
  DEPENDS:=+libpthread +libstdcpp +kmod-sgx544-km
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/sgx544-um/description
	SGX544 userspace binary files
endef
 
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/bin
	mkdir -p $(PKG_BUILD_DIR)/lib
	$(CP) fbdev/bin/* $(PKG_BUILD_DIR)/bin
	$(CP) -d fbdev/lib/* $(PKG_BUILD_DIR)/lib
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/InstallDev
	mkdir -p $(1)/usr/include
	mkdir -p $(1)/usr/lib

	$(CP) fbdev/include/* $(1)/usr/include
	$(CP) fbdev/lib/* $(1)/usr/lib
endef

define Package/sgx544-um/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/bin/* $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) -d $(PKG_BUILD_DIR)/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,sgx544-um))

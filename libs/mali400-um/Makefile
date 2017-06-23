include $(TOPDIR)/rules.mk

# Name and release number of this package
PKG_NAME:=mali400-um
PKG_VERSION:=1.1.2
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk
 
define Package/mali400-um
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=mali400 userspace libs
  DEPENDS:=+libpthread +librt +kmod-mali400-km
endef

ifeq ($(TARGET_BOARD_PLATFORM), astar)
	TARGET = R16
endif
ifeq ($(TARGET_BOARD_PLATFORM), azalea)
	TARGET = R40
endif

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/mali400-um/description
	mali400 userspace libraries
endef
 
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -u -d ./$(TARGET)/fbdev/lib/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/InstallDev
	mkdir -p $(PKG_INSTALL_DIR)
	mkdir -p $(PKG_INSTALL_DIR)/usr/include/
	mkdir -p $(PKG_INSTALL_DIR)/usr/lib/
	$(CP) -u ./$(TARGET)/fbdev/include/* $(PKG_INSTALL_DIR)/usr/include/
	$(CP) -u -d ./$(TARGET)/fbdev/lib/* $(PKG_INSTALL_DIR)/usr/lib/
	mkdir -p $(1)/usr/include
	mkdir -p $(1)/usr/lib
	$(CP) -u ./$(TARGET)/fbdev/include/* $(1)/usr/include/
	$(CP) -u -d ./$(TARGET)/fbdev/lib/* $(1)/usr/lib
endef

define Package/mali400-um/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) -u -d ./$(TARGET)/fbdev/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,mali400-um))

include $(TOPDIR)/rules.mk

PKG_NAME:=boot-play
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
HAVE_SOURCE := yes

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:=+tinymp3
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=boot play
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/media
	$(INSTALL_BIN) play.init $(1)/etc/init.d/play
	$(INSTALL_DATA) media/* $(1)/media/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

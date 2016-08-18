#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cellphoneuxdemo
PKG_VERSION:=
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=e14dc09754a20c11e88ddbfcf3fdb04f

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
CMAKE_INSTALL:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/cellphoneuxdemo
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl
  TITLE:=cellphoneuxdemo
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/cellphoneuxdemo/description
  The minigui test case
endef

CMAKE_OPTIONS += -DCMAKE_MODULE_PATH=$(STAGING_DIR)/usr/share/minigui/cmake/Modules

define Package/cellphoneuxdemo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) \
		$(PKG_BUILD_DIR)/same/same \
		$(1)/usr/bin/same
	$(CP) \
		$(PKG_BUILD_DIR)/notebook/notebook \
		$(1)/usr/bin/notebook
endef

$(eval $(call BuildPackage,cellphoneuxdemo))

#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libminigui-gpl
PKG_VERSION:=3.0.12
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=c09dbc378344945460b5142956d2cd34

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/libminigui-gpl
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +zlib +libpthread +libpng +libjpeg +libfreetype +minigui-res-be +kmod-sunxi-disp
  TITLE:=libminigui-gpl
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/libminigui-gpl/description
  The ligminigui library of GUI Core
endef

define Package/libminigui-gpl/config
choice
prompt "Preferred Minigui Run Mode"
	default MINIGUI_THS
	help
	Select the preferred minigui running mode.
config MINIGUI_THS
	bool "ths"
config MINIGUI_PROC
	bool "proc"
endchoice
endef

CONFIGURE_ARGS += \
	--disable-pcxvfb \
	--disable-splash \
	--disable-screensaver

ifeq ($(CONFIG_MINIGUI_PROC),y)
CONFIGURE_ARGS += --enable-procs
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/minigui \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libminigui_*.{a,la,so*} \
		$(1)/usr/lib/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/minigui.pc \
		$(1)/usr/lib/pkgconfig/

	$(INSTALL_DIR) $(1)/usr/share/minigui/cmake/Modules
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/minigui/cmake/Modules/*.cmake \
		$(1)/usr/share/minigui/cmake/Modules/
endef

define Package/libminigui-gpl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	$(CP) \
		./MiniGUI.cfg \
		$(1)/usr/local/etc/
endef

$(eval $(call BuildPackage,libminigui-gpl))

#
# Copyright (C) 2008-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gst-omx
PKG_VERSION:=1.10.4
PKG_RELEASE:=1
PKG_SOURCE:=gst-omx-$(PKG_VERSION).tar.xz
include $(BUILD_DIR)/package.mk

define Package/gst-omx
	SECTION:=multimedia
	CATEGORY:=Multimedia
	TITLE:=gst openmax lib
	DEPENDS:=+glib2 +gstreamer1 +gstreamer1-plugins-base +libgst1video +libgst1audio +libgst1pbutils
endef


define Package/gst-omx/description
 GStreamer openmax API
endef

define Build/Configure
	cd $(PKG_BUILD_DIR);./autogen.sh --host=arm-openwrt-linux --prefix=$(PKG_BUILD_DIR) --target=arm-openwrt-linux --disable-gtk-doc --with-omx-target=generic
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR) install
endef

define Package/gst-omx/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) -rf $(PKG_BUILD_DIR)/lib/gstreamer-1.0 $(1)/usr/lib
	mkdir -p $(1)/etc/xdg
	$(CP) $(PKG_BUILD_DIR)/config/bellagio/gstomx.conf $(1)/etc/xdg/
endef

$(eval $(call BuildPackage,gst-omx))

#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

WIRELESS_MENU:=Wireless Drivers

define KernelPackage/net-rtl8188eu
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=RTL8188EU support (staging)
  DEPENDS:=@USB_SUPPORT +@DRIVER_WEXT_SUPPORT +r8188eu-firmware +kmod-usb-core
#  KCONFIG:=\
#	CONFIG_STAGING=y \
#	CONFIG_R8188EU \
#	CONFIG_88EU_AP_MODE=y \
#	CONFIG_88EU_P2P=n
  FILES:=$(LINUX_DIR)/drivers/net/wireless/rtl8188eu/8188eu.ko
  AUTOLOAD:=$(call AutoProbe,8188eu)
endef

define KernelPackage/net-rtl8188eu/description
 Kernel modules for RealTek RTL8188EU support
endef

$(eval $(call KernelPackage,net-rtl8188eu))

define KernelPackage/net-rtl8723bs
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=RTL8723BS support (staging)
  DEPENDS:=@USB_SUPPORT +@DRIVER_WEXT_SUPPORT +r8723bs-firmware
#  KCONFIG:=\
#	CONFIG_STAGING=y \
#	CONFIG_R8723BS \
#	CONFIG_23BS_AP_MODE=y \
#	CONFIG_23BS_P2P=n
  FILES:=$(LINUX_DIR)/drivers/net/wireless/rtl8723bs/8723bs.ko
  AUTOLOAD:=$(call AutoProbe,8723bs)
endef

define KernelPackage/net-rtl8723bs/description
 Kernel modules for RealTek RTL8723BS support
endef

$(eval $(call KernelPackage,net-rtl8723bs))

define KernelPackage/cfg80211
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=cfg80211 support (staging)
  DEPENDS:=
  FILES:=$(LINUX_DIR)/net/wireless/cfg80211.ko
  AUTOLOAD:=$(call AutoProbe,cfg80211)
endef

define KernelPackage/cfg80211/description
 Kernel modules for CFG80211 support
endef

$(eval $(call KernelPackage,cfg80211))

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
  KCONFIG:=\
	CONFIG_STAGING=y \
	CONFIG_R8188EU \
	CONFIG_88EU_AP_MODE=y \
	CONFIG_88EU_P2P=n
  FILES:=$(LINUX_DIR)/drivers/staging/rtl8188eu/r8188eu.ko
  AUTOLOAD:=$(call AutoProbe,r8188eu)
endef

define KernelPackage/net-rtl8188eu/description
 Kernel modules for RealTek RTL8188EU support
endef

$(eval $(call KernelPackage,net-rtl8188eu))

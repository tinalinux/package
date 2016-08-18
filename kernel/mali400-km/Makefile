#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=mali400-km
PKG_RELEASE:=2

include $(BUILD_DIR)/package.mk

define KernelPackage/mali400-km
  SUBMENU:=Video Support
  TITLE:=mali400 device driver
  DEPENDS:= +kmod-sunxi-disp
  FILES:=$(LINUX_DIR)/modules/gpu/mali400/kernel_mode/driver/src/devicedrv/mali/mali.ko
  AUTOLOAD:=$(call AutoLoad,50,mali)
  KCONFIG:=
endef

define KernelPackage/mali400-km/description
 Kernel module for mali400-km support.
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(LINUX_DIR)/modules/gpu/mali400/kernel_mode/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)/modules/gpu/mali400/kernel_mode/driver/src/devicedrv/mali" \
	KDIR=$(LINUX_DIR) USING_UMP=0 $(MAKE_OPTS)
endef

$(eval $(call KernelPackage,mali400-km))

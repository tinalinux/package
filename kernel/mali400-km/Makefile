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

# the GPU driver will check whether the TARGET_PLATFORM
# is zero when it's compiled. we need to shield the influence of
# environmental variables on TARGET_PLATFORM to prevent the checking error.
TARGET_PLATFORM=

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
	USING_DT=1 \
	BUILD=release \
	USING_UMP=0

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -u $(LINUX_DIR)/modules/gpu/mali400/kernel_mode/* $(PKG_BUILD_DIR)/
endef

# -j2 --Using multi-core compiler
define Build/Compile
	$(MAKE) -j2 -C "$(LINUX_DIR)/modules/gpu/mali400/kernel_mode/driver/src/devicedrv/mali" \
	KDIR=$(LINUX_DIR) $(MAKE_OPTS)
endef

$(eval $(call KernelPackage,mali400-km))

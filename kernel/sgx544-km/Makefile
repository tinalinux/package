#
# Copyright (C) 2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=sgx544-km
PKG_RELEASE:=2

include $(BUILD_DIR)/package.mk

define KernelPackage/sgx544-km
	SUBMENU:=Video Support
	TITLE:=sgx544 device driver
	DEPENDS:= +kmod-sunxi-disp
	FILES:=$(LINUX_DIR)/modules/gpu/sgx544/linux/kernel_mode/eurasia_km/eurasiacon/binary2_sunxi_linux_release/target/pvrsrvkm.ko
	FILES+=$(LINUX_DIR)/modules/gpu/sgx544/linux/kernel_mode/eurasia_km/eurasiacon/binary2_sunxi_linux_release/target/dc_sunxi.ko
	AUTOLOAD:=$(call AutoLoad,20,pvrsrvkm,1)
	AUTOLOAD+=$(call AutoLoad,25,dc_sunxi,1)
	KCONFIG:=
endef

define KernelPackage/sgx544-km/description
 Kernel module for sgx544-km support.
endef

MAKE_OPTS:= \
	ARCH="$(LINUX_KARCH)" \
	CROSS_COMPILE="$(TARGET_CROSS)" \
	SUBDIRS="$(PKG_BUILD_DIR)"

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(LINUX_DIR)/modules/gpu/sgx544/linux/kernel_mode/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C "$(LINUX_DIR)/modules/gpu/sgx544/linux/kernel_mode/eurasia_km/eurasiacon/build/linux2/sunxi_linux" KERNELDIR=$(LINUX_DIR) $(MAKE_OPTS)
endef

$(eval $(call KernelPackage,sgx544-km))

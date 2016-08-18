include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=sayeye
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(BUILD_DIR)/package.mk

define Package/sayeye
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:= +libutils +libcutils +libsysutils
  TITLE:=sayeye from Android L
endef

define Package/$(PKG_NAME)/description
  sayeye form Android L
endef


define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) ./src $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

ifeq ($(TARGET_BOARD_PLATFORM), kylin)
TARGET_CFLAGS   += -DSUN9IW1P1 -D__arm__
endif
ifeq ($(TARGET_BOARD_PLATFORM), astar)
TARGET_CFLAGS   += -DSUN8IW5P1 -D__arm__
endif
ifeq ($(TARGET_BOARD_PLATFORM), octopus)
TARGET_CFLAGS   += -DSUN8IW6P1 -D__arm__
endif
ifeq ($(TARGET_BOARD_PLATFORM), tulip)
TARGET_CFLAGS   += -DSUN50IW1P1 -D__aarch64__
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/sayeye/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/bin
	$(INSTALL_BIN) ./sayeye.init $(1)/etc/init.d/sayeye
endef

$(eval $(call BuildPackage,sayeye))

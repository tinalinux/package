include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=tplayerdemo
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=tplayerdemo which use tplayer
  DEPENDS:=+libcedarx2.7 \
		+alsa-utils
endef

define Package/$(PKG_NAME)/description
	CedarX2.7 tplayerdemo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
ifeq ($(TARGET_BOARD_PLATFORM),astar)
	$(CP) r16_tplayerdemo.sh $(PKG_BUILD_DIR)/tplayerdemo
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
endif

ifeq ($(TARGET_BOARD_PLATFORM),octopus)
	$(CP) r58_tplayerdemo.sh $(PKG_BUILD_DIR)/tplayerdemo
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
endif

ifeq ($(TARGET_BOARD_PLATFORM),azalea)
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
	$(CP) r40_tplayerdemo.sh $(PKG_BUILD_DIR)/tplayerdemo
endif

ifeq ($(TARGET_BOARD_PLATFORM),tulip)
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
	$(CP) r18_tplayerdemo.sh $(PKG_BUILD_DIR)/tplayerdemo
endif

ifeq ($(TARGET_BOARD_PLATFORM),sitar)
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
	$(CP) c600_tplayerdemo.sh $(PKG_BUILD_DIR)/tplayerdemo
endif

ifeq ($(TARGET_BOARD_PLATFORM),cello)
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
	$(CP) G102_tplayerdemo.sh $(PKG_BUILD_DIR)/tplayerdemo
endif

ifeq ($(TARGET_BOARD_PLATFORM),banjo)
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
	$(CP) MR100_tplayerdemo.sh $(PKG_BUILD_DIR)/tplayerdemo
endif

endef

define Build/Compile

	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		TARGET_BOARD="$(TARGET_BOARD_PLATFORM)"
endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tplayerdemo					 $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/realtplayerdemo					 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

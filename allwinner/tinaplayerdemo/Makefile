include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=tinaplayerdemo
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/tinaplayerdemo
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=CedarX2.6 tina player utils for softwinner
  DEPENDS:=+liballwinner \
		+alsa-utils \
		+nativepower
endef

define Package/tinaplayerdemo/description
	CedarX2.6 tina player
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
ifeq ($(TARGET_BOARD_PLATFORM),astar)
	$(CP) r16_tinaplayerdemo.sh $(PKG_BUILD_DIR)/tinaplayerdemo
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
endif

ifeq ($(TARGET_BOARD_PLATFORM),octopus)
	$(CP) r58_tinaplayerdemo.sh $(PKG_BUILD_DIR)/tinaplayerdemo
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
endif

ifeq ($(TARGET_BOARD_PLATFORM),azalea)
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
	$(CP) r40_tinaplayerdemo.sh $(PKG_BUILD_DIR)/tinaplayerdemo
endif

endef

define Build/Compile

	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef


define Package/tinaplayerdemo/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinaplayerdemo					 $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/realtinaplayerdemo					 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,tinaplayerdemo))

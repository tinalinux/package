Package/r8188eu-firmware = $(call Package/firmware-default,RealTek RTL8188EU firmware)
define Package/r8188eu-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware/rtlwifi
#	#$(CP) \
#		$(PKG_BUILD_DIR)/rtlwifi/rtl8188eufw.bin \
#		$(1)/lib/firmware/rtlwifi
endef
$(eval $(call BuildPackage,r8188eu-firmware))

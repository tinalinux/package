Package/xr819-firmware = $(call Package/firmware-default,Xradio xr819 firmware)
define Package/xr819-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/xr819/*.bin \
		$(1)/lib/firmware/
endef
$(eval $(call BuildPackage,xr819-firmware))

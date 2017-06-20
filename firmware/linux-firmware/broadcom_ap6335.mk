Package/ap6335-firmware = $(call Package/firmware-default,Broadcom AP6335 firmware)
define Package/ap6335-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6335/*.bin \
		$(1)/lib/firmware/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6335/*.hcd \
		$(1)/lib/firmware/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6335/nvram_ap6335.txt \
		$(1)/lib/firmware/nvram.txt
endef
$(eval $(call BuildPackage,ap6335-firmware))

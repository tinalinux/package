Package/ap6356s-firmware = $(call Package/firmware-default,Broadcom AP6356S firmware)
define Package/ap6356s-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6356s/*.bin \
		$(1)/lib/firmware/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6356s/*.hcd \
		$(1)/lib/firmware/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6356s/nvram_ap6356s.txt \
		$(1)/lib/firmware/nvram.txt
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6356s/config.txt \
		$(1)/lib/firmware/config.txt
endef
$(eval $(call BuildPackage,ap6356s-firmware))

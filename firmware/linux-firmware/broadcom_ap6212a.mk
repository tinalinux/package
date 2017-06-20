Package/ap6212a-firmware = $(call Package/firmware-default,Broadcom AP6212A firmware)
define Package/ap6212a-firmware/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6212a/*.bin \
		$(1)/lib/firmware/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6212a/*.hcd \
		$(1)/lib/firmware/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6212a/nvram_ap6212a.txt \
		$(1)/lib/firmware/nvram.txt
endef
$(eval $(call BuildPackage,ap6212a-firmware))

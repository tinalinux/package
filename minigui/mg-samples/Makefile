#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mg-samples
PKG_VERSION:=3.0.12
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=e14dc09754a20c11e88ddbfcf3fdb04f

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/mg-samples
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl
  TITLE:=mg-smaples
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/mg-samples/description
  The minigui test case
endef

CONFIGURE_ARGS += \
	--with-lang=zhcn

define Package/mg-samples/install
	$(INSTALL_DIR) $(1)/usr/bin/ctrl_dlg
	$(INSTALL_DIR) $(1)/usr/bin/ctrl_dlg/res
	$(CP) \
		$(PKG_BUILD_DIR)/ctrl_dlg/ctrl_dlg \
		$(1)/usr/bin/ctrl_dlg/
	$(CP) \
		$(PKG_BUILD_DIR)/ctrl_dlg/*.jpg \
		$(1)/usr/bin/ctrl_dlg/
	$(CP) \
		$(PKG_BUILD_DIR)/ctrl_dlg/res/*.{gif,ico,bmp,jpg} \
		$(1)/usr/bin/ctrl_dlg/res/

	$(INSTALL_DIR) $(1)/usr/bin/dbuff
	$(INSTALL_DIR) $(1)/usr/bin/dbuff/res
	$(CP) \
		$(PKG_BUILD_DIR)/dbuff \
		$(1)/usr/bin/dbuff/
	$(CP) \
		$(PKG_BUILD_DIR)/dbuff/res/*.png \
		$(1)/usr/bin/dbuff/res/

	$(INSTALL_DIR) $(1)/usr/bin/font_text
	$(CP) \
		$(PKG_BUILD_DIR)/font_text/*.png \
		$(1)/usr/bin/dbuff/
	$(CP) \
		$(PKG_BUILD_DIR)/font_text/font_text \
		$(1)/usr/bin/font_text/

	$(INSTALL_DIR) $(1)/usr/bin/graphics
	$(INSTALL_DIR) $(1)/usr/bin/graphics/res
	$(CP) \
		$(PKG_BUILD_DIR)/graphics/res/*.bmp \
		$(1)/usr/bin/graphics/res/
	$(CP) \
		$(PKG_BUILD_DIR)/graphics/graphics \
		$(1)/usr/bin/graphics/
	$(CP) \
		$(PKG_BUILD_DIR)/graphics/graphicsex \
		$(1)/usr/bin/graphics/

	$(INSTALL_DIR) $(1)/usr/bin/housekeeper
	$(INSTALL_DIR) $(1)/usr/bin/housekeeper/res
	$(CP) \
		$(PKG_BUILD_DIR)/housekeeper/housekeeper \
		$(1)/usr/bin/housekeeper/
	$(CP) \
		$(PKG_BUILD_DIR)/housekeeper/res/*.gif \
		$(1)/usr/bin/housekeeper/res/

	$(INSTALL_DIR) $(1)/usr/bin/lf_skin
	#$(INSTALL_DIR) $(1)/usr/bin/lf_skin/res
	$(CP) \
		$(PKG_BUILD_DIR)/lf_skin/lf_skin \
		$(1)/usr/bin/lf_skin/
	$(CP) \
		$(PKG_BUILD_DIR)/lf_skin/res \
		$(1)/usr/bin/lf_skin/

	$(INSTALL_DIR) $(1)/usr/bin/look_feel
	$(INSTALL_DIR) $(1)/usr/bin/look_feel/res
	$(CP) \
		$(PKG_BUILD_DIR)/look_feel/look_feel \
		$(1)/usr/bin/look_feel/
	$(CP) \
		$(PKG_BUILD_DIR)/look_feel/res/*.{bmp,ico,png,bmp} \
		$(1)/usr/bin/look_feel/res/

ifeq ($(CONFIG_MINIGUI_PROC),y)
	$(INSTALL_DIR) $(1)/usr/bin/mginit
	$(INSTALL_DIR) $(1)/usr/bin/mginit/res
	$(CP) \
		$(PKG_BUILD_DIR)/mginit/mginit \
		$(1)/usr/bin/mginit/
	$(CP) \
		$(PKG_BUILD_DIR)/mginit/mginit.rc \
		$(1)/usr/bin/mginit/
	$(CP) \
		$(PKG_BUILD_DIR)/mginit/res/*.{png,gif} \
		$(1)/usr/bin/mginit/res/
endif

	$(INSTALL_DIR) $(1)/usr/bin/minesweeper
	$(INSTALL_DIR) $(1)/usr/bin/minesweeper/res
	$(CP) \
		$(PKG_BUILD_DIR)/minesweeper/minesweeper \
		$(1)/usr/bin/minesweeper/
	$(CP) \
		$(PKG_BUILD_DIR)/minesweeper/res/*.bmp \
		$(1)/usr/bin/minesweeper/res/

	$(INSTALL_DIR) $(1)/usr/bin/notebook
	$(INSTALL_DIR) $(1)/usr/bin/notebook/res
	$(CP) \
		$(PKG_BUILD_DIR)/notebook/notebook \
		$(1)/usr/bin/notebook/
	$(CP) \
		$(PKG_BUILD_DIR)/notebook/res/*.ico \
		$(1)/usr/bin/notebook/res/

	$(INSTALL_DIR) $(1)/usr/bin/same
	$(INSTALL_DIR) $(1)/usr/bin/same/res
	$(CP) \
		$(PKG_BUILD_DIR)/same/same \
		$(1)/usr/bin/same/
	$(CP) \
		$(PKG_BUILD_DIR)/same/res/*.gif \
		$(1)/usr/bin/same/res/

	$(INSTALL_DIR) $(1)/usr/bin/src
	$(INSTALL_DIR) $(1)/usr/bin/src/res
	$(CP) \
		$(PKG_BUILD_DIR)/src/{animation,menubutton,edit,bitblt,scrollview,flame,monthcal,bmpbkgnd,fminigui,scrollwnd,scrollwnd2,mycontrol,button,gridview,newtoolbar,gridview_ext,capture,simplekey,nr_window,caretdemo,painter,helloworld,spinbox,combobox,palette,iconview,palette_ex,static,coolbar,progressbar,input,stretchblt,createicon,propsheet,listbox,timeeditor,cursordemo,listview,scalerbmp,trackbar,scrnsaver,dialogbox,loadbmp,treeview,scrollbar,scrollbar_ctrl,drawicon} \
		$(1)/usr/bin/src/
	$(CP) \
		$(PKG_BUILD_DIR)/src/res/*.{gif,jpg,png} \
		$(1)/usr/bin/src/res/

endef

$(eval $(call BuildPackage,mg-samples))

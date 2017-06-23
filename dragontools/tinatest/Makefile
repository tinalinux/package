include $(TOPDIR)/rules.mk

PKG_NAME:=tinatest
PKG_VERSION:=1
PKG_RELEASE:=2

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/tinatest
  SECTION:=dragon
  CATEGORY:=DragonTools
  TITLE:=test platform for function/specification/stress
  MENU:=1
  DEPENDS := +libjson-c +TINATEST_SYS_GLOBAL_INFO_COLLECTD:collectd
  include Depends.mk
endef

define Package/tinatest/config
	source "$(SOURCE)/Config.in"
endef

define Package/tinatest/description
  Tinatest is a test platform for function/specification/stress test.
  It is customized in menuconfig and give birth to tinatest.json which control
  how tinatest run.
  It will also collect system info by collectd while running testcases.
  Tinatest is a strong tool to test applications but also to test tinalinux.
  You can use it to daily automated test, production function test and so on.
endef

PREFIX := $(PKG_BUILD_DIR)/src/out
TTLIB := -ltinatest -L$(PREFIX)/lib -ljson-c -pthread -ldl
TTINCLUDE := -I$(PREFIX)/include -I$(PKG_BUILD_DIR)/src/core
export CP MAKE TTLIB

define Build/Prepare
	mkdir -p $(PREFIX)/include
	mkdir -p $(PREFIX)/bin
	mkdir -p $(PREFIX)/lib
	mkdir -p $(PREFIX)/lib/tt-module
	$(CP) ./src $(PKG_BUILD_DIR)
	$(CP) ./testcase $(PKG_BUILD_DIR)

ifeq ($(CONFIG_TINATEST_CUSTOM),y)
	grep "^CONFIG_TINATEST.*" $(TOPDIR)/.config > $(PKG_BUILD_DIR)/.config
	./tools/convert_json.sh $(PKG_BUILD_DIR) > $(PKG_BUILD_DIR)/src/tinatest.json
endif

endef

include Modules.mk

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) $(TTINCLUDE)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		BUILD_DIR="$(PKG_BUILD_DIR)" \
		MCOLLECTD_MODULES="$(MCOLLECTD_MODULES)" \
		OUTLOG_MODULES="$(OUTLOG_MODULES)" \
		PREFIX="$(PREFIX)" \
		all

	$(MAKE) -C $(PKG_BUILD_DIR)/testcase \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) $(TTINCLUDE)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(TTLIB)" \
		BUILD_DIR="$(PKG_BUILD_DIR)" \
		ISCUSTOM="$(CONFIG_TINATEST_CUSTOM)" \
		all

endef

define Package/tinatest/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib/tt-module
	# library && tiantest && modules
	$(INSTALL_BIN)  $(PREFIX)/bin/* $(1)/usr/bin
	$(INSTALL_DATA) $(PREFIX)/lib/*.so $(1)/usr/lib
	$(INSTALL_DATA) $(PREFIX)/include/* $(1)/usr/include
	$(INSTALL_DATA) $(PREFIX)/lib/tt-module/* $(1)/usr/lib/tt-module
	$(LN) /usr/bin/tinatest $(1)/usr/bin/tt
	$(CP) $(PKG_BUILD_DIR)/src/tinatest.json $(1)/etc
	# testcases
	[ -n "$$(shell ls $(PKG_BUILD_DIR)/testcase/bin/*)" ] && \
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/testcase/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,tinatest))

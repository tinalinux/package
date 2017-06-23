TESTCASES_ALL := $(shell find -maxdepth 2 -name "list" | xargs cat)
TESTCASES_ENABLE := $(shell \
	for one in $(TESTCASES_ALL); \
	do \
		echo $${one} | sed '{s/[a-z]/\u&/g; s/[^ [:alnum:]]/_/g}' | \
		xargs -I {} grep "CONFIG_TINATEST{}_ENABLE" $(BUILD_DIR)/.config 1>/dev/null 2>&1 && \
		echo $${one}; \
	done \
)

ifeq ($(ISCUSTOM),)
TESTCASES_ENABLE := $(shell cat ./base/list)
endif

PREFIX := $(BUILD_DIR)/testcase/bin

.PHONY: all

all: preinit $(TESTCASES_ENABLE)

define BuildTestcase

$(1):
ifneq ($$(wildcard .$(1)/Makefile),)
	@$(MAKE) -C .$(1) \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)" \
		PREFIX="$(PREFIX)" \
		all
else
	@for one in `ls .$(1)`; \
	do \
		case $$$${one} in \
			*.c) \
				$(CC) $(CFLAGS) $(LDFLAGS) .$(1)/$$$${one} -o $(PREFIX)/`echo $$$${one%.*}`; \
				;; \
			*.cpp) \
				$(CXX) $(CFLAGS) $(LDFLAGS) .$(1)/$$$${one} -o $(PREFIX)/`echo $$$${one%.*}`; \
				;; \
			*.sh) \
				cp .$(1)/$$$${one} $(PREFIX); \
				;; \
		esac \
	done
endif

endef

$(foreach one,$(TESTCASES_ENABLE),$(eval $(call BuildTestcase,$(one))))

preinit:
	@rm -rf $(PREFIX)
	@mkdir -p $(PREFIX)

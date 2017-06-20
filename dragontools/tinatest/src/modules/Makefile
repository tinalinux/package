.PHONY: all mcollectd outlog

all: mcollectd outlog

mcollectd:
ifneq ($(MCOLLECTD_MODULES),)
	@$(MAKE) -C ./mcollectd \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)" \
		BUILD_DIR="$(BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		$(MCOLLECTD_MODULES)
endif

outlog:
ifneq ($(OUTLOG_MODULES),)
	@$(MAKE) -C ./outlog \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)" \
		BUILD_DIR="$(BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		$(OUTLOG_MODULES)
endif

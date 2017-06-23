TARGET := tinatest
.PHONY: all build-library build-modules $(TARGET)

$(TARGET): build-library
	@$(MAKE) -C $(BUILD_DIR)/src/core \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(TTLIB)" \
		BUILD_DIR="$(BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		all

all: build-library build-modules $(TARGET)

build-modules: build-library
	@$(MAKE) -C $(BUILD_DIR)/src/modules \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS) -fPIC -shared" \
		LDFLAGS="$(LDFLAGS) $(TTLIB)" \
		BUILD_DIR="$(BUILD_DIR)" \
		MCOLLECTD_MODULES="$(MCOLLECTD_MODULES)" \
		OUTLOG_MODULES="$(OUTLOG_MODULES)" \
		PREFIX="$(PREFIX)" \
		all

build-library:
	@$(MAKE) -C $(BUILD_DIR)/src/library \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS) $(INCLUDES)" \
		LDFLAGS="$(LDFLAGS) $(LIBS)" \
		BUILD_DIR="$(BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		all

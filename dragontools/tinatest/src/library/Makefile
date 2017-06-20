TARGET := libtinatest.so

SUBLIBS := \
	mcollectd \
	mjson \
	sysinfo \
	outlog \
	sync \
	fileops \

.PHONY: all $(SUBLIBS)

all: $(SUBLIBS) $(TARGET)

INCLUDES := $(addprefix -I../,$(SUBLIBS))

$(SUBLIBS):
	@$(MAKE) -C $(BUILD_DIR)/src/library/$@ \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS) -c -Wall -fPIC $(INCLUDES)" \
		BUILD_DIR="$(BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		all

$(TARGET): $(SUBLIBS)
	$(CC) -fPIC -shared $(LDFLAGS) $(foreach var,$(SUBLIBS),$(addprefix ./$(var)/,$(addsuffix .o,$(var)))) -o $(PREFIX)/lib/$@
	# build mjson_fetch for scripts api
	@$(MAKE) -C $(BUILD_DIR)/src/library/mjson \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(TTLIB)" \
		PREFIX="$(PREFIX)" \
		mjson_fetch

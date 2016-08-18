target			= healthd

INCLUDES        += -I. -I./include
LIBS            := -lpthread -lcutils -lminui -lpng -lutils
#LIBS            := -lpthread -lcutils -lutils

SRCS =healthd.cpp \
      healthd_mode_tina.cpp \
      healthd_mode_charger.cpp \
      BatteryMonitor.cpp

install:$(target)
	@mkdir -p $(CONFIG_PREFIX)/bin
	@cp $(target) $(CONFIG_PREFIX)/bin

$(target): $(SRCS)
	$(CXX) $(INCLUDES) $(LDFLAGS) $^ -o $@ $(LIBS)

####################################################################
all: install
clean:
	rm -rf $(target)

target			= healthd

INCLUDES        += -I. -I./include -I../libbatinfo/include -DCONFIG_TLOG_LEVEL=OPTION_TLOG_LEVEL_WARNING
LIBS            := -L../libbatinfo -lpthread -lcutils -lminui -lpng -lutils -lbatinfo -ldbus-1 -luci
#LIBS            := -lpthread -lcutils -lutils

SRCS =healthd.cpp \
      healthd_mode_tina.cpp \
      healthd_mode_charger.cpp \
      BatteryMonitor.cpp

OBJS = $(SRCS:.cpp=.o)

install:$(target)
	@mkdir -p $(CONFIG_PREFIX)/bin
	@cp $(target) $(CONFIG_PREFIX)/bin

%.o: %.cpp
	$(CXX) $(CFLAGS) $(INCLUDES) -c -o $@ $<

$(target): $(OBJS)
	$(CXX) $(INCLUDES) $(LDFLAGS) $(LIBS) $^ -o $@

####################################################################
all: install
clean:
	rm -rf $(target) $(OBJS)

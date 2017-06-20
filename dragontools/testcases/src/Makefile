dirs := \
	memtester \
	sdcardtester \
	emmctester \
	nandtester \
	keytester \
	tptester \
	udisktester \
	rtctester \
	pmutester \
	host1tester \
	host2tester \
	otgtester \
	irtester \
	wifitester \
	mictester \
	ledstester \
	hdmitester \
	ethtester \
	bttester \
	satatester \
	headphonetester \
	example

subdirs:
	$(foreach dir, $(dirs), $(MAKE) -C $(dir) all || exit "$$?";)

install:

all: subdirs install

clean:
	echo "clean"

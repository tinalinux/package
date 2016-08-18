dirs := \
	memtester \
	mmctester \
	emmctester \
	nandtester \
	keytester \
	udisktester \
	rtctester \
	pmutester \
	host1tester \
	host2tester \
	otgtester \
	irtester \
	gsensortester \
	wifitester \
	compasstester \
	hdmitester \
	gyrotester \
	mictester \
	example

	#bluetooth \
	mictester \
	rtk_hciattach \
	cameratester \

subdirs:
	$(foreach dir, $(dirs), $(MAKE) -C $(dir) all;)

install:
	mkdir -p $(INSTALL_PREFIX)/usr/bin
	cp *.sh $(INSTALL_PREFIX)/usr/bin

all: subdirs install

clean:
	echo "clean"

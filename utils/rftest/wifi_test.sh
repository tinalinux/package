#!/bin/sh
rmmod bcmdhd
sleep 2
insmod /lib/modules/3.10.65/bcmdhd.ko iface_name=wlan0 firmware_path=/etc/rftest/fw_bcm43438a0_mfg.bin nvram_path=/lib/firmware/nvram.txt
sleep 2
ifconfig wlan0 up
sleep 1
./wl ver

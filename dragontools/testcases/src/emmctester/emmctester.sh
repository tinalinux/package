#!/bin/sh

source send_cmd_pipe.sh
source script_parser.sh

BOOT_TYPE=-1
isnand=`cat /proc/cmdline | grep nanda`
if [ ! -z $isnand ]; then
	BOOT_TYPE=0
	SEND_CMD_PIPE_FAIL_EX $3 " no eMMC found"
fi

emmcblk=`ls /sys/devices/soc/sdc2/mmc_host/*/*/block/ | grep mmcblk*`
if [ "$emmcblk" = "mmcblk0" ]; then
	if [ -b "/dev/mmcblk0" ]; then
		name=`cat /sys/devices/soc/sdc2/mmc_host/*/*/name`
		SEND_CMD_PIPE_OK_EX $3 "$name"
		exit 1
	fi
fi

if [ "$emmcblk" = "mmcblk1" ]; then
	if [ -b "/dev/mmcblk1" ]; then
		name=`cat /sys/devices/soc/sdc2/mmc_host/*/*/name`
		SEND_CMD_PIPE_OK_EX $3 "$name"
		exit 1
	fi
fi

SEND_CMD_PIPE_FAIL_EX $3 "eMMC not init"
echo "emmc test fail!!"

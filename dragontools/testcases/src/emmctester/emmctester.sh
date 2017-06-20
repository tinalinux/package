#!/bin/sh

source send_cmd_pipe.sh
source script_parser.sh
source sysinfo.sh

boot_type=`get_boot_type`
case "$boot_type" in
	nand|nor-flash)
		SEND_CMD_PIPE_FAIL_EX $3 " no eMMC found"
		exit 1
		;;
	emmc)
		if [ -b "/dev/mmcblk0" ]; then
			SEND_CMD_PIPE_OK_EX $3 "boot from emmc"
			exit 1
		fi
		;;
	sdcard)
		if [ -b "/dev/mmcblk1" ]; then
			SEND_CMD_PIPE_OK_EX $3 "boot from sdcard"
			exit 1
		fi
		;;
esac

SEND_CMD_PIPE_FAIL_EX $3 "eMMC not init"
echo "emmc test fail!!"

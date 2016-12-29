#!/bin/sh

source send_cmd_pipe.sh
source script_parser.sh

emmcblk=`ls /sys/devices/soc/sdc2/mmc_host/*/*/block/ | grep mmcblk*`
count=0
if [ "$emmcblk" = "mmcblk0" ]; then
	while true; do
		let count=$count+1
		if [ -b "/dev/mmcblk1" ]; then
			name=`cat /sys/devices/soc/sdc0/mmc_host/*/*/name`
			SEND_CMD_PIPE_OK_EX $3 "$name"
			exit 1
		fi
		if [ $count -gt 60 ]; then
			break
		fi
		sleep 1
	done
else
	while true; do
		let count=$count+1
		if [ -b "/dev/mmcblk0" ]; then
			name=`cat /sys/devices/soc/sdc0/mmc_host/*/*/name`
			SEND_CMD_PIPE_OK_EX $3 "$name"
			exit 1
		fi
		if [ $count -gt 60 ]; then
			break
		fi
		sleep 1
	done
fi

SEND_CMD_PIPE_FAIL_EX $3 "TF-card not found"
echo "TF-card test fail!!"

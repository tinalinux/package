#!/bin/sh

source send_cmd_pipe.sh
source script_parser.sh

count=0
while true; do
	blk=`ls /sys/devices/soc/sata/ata1/host0/*/*/block/`
	if [ -b "/dev/$blk" ]; then
		name=`cat /sys/devices/soc/sata/ata1/host0/*/*/model`
		SEND_CMD_PIPE_OK_EX $3 "$name"
		exit 1
	fi
	if [ $count -gt 60 ]; then
		break
	fi
	let count=$count+1
	sleep 1
done

SEND_CMD_PIPE_FAIL_EX $3 "sata not found"
echo "sata test fail!!"

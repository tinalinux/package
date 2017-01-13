#!/bin/sh

source send_cmd_pipe.sh
source script_parser.sh
source sysinfo.sh

count=0
boot_type=`get_boot_type`
case  "${boot_type}" in
	emmc)
		while true; do
			let count=$count+1
			if [ -b "/dev/mmcblk1" ]; then
				SEND_CMD_PIPE_OK_EX $3 "boot from ${boot_type}"
				exit 1
			fi
			if [ $count -gt 60 ]; then
				SEND_CMD_PIPE_FAIL_EX $3 "TF-card not found"
				echo "TF-card test fail!!"
			fi
			sleep 1
		done
		;;
	*)
		while true; do
			let count=$count+1
			if [ -b "/dev/mmcblk0" ]; then
				SEND_CMD_PIPE_OK_EX $3 "boot from ${boot_type}"
				exit 1
			fi
			if [ $count -gt 60 ]; then
				SEND_CMD_PIPE_FAIL_EX $3 "TF-card not found"
				echo "TF-card test fail!!"
			fi
			sleep 1
		done
		;;
esac


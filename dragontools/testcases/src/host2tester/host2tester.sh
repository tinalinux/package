#!/bin/sh

source send_cmd_pipe.sh
source sysinfo.sh

board="$(get_target_board | awk '{print $1}')"

while true; do
	case "$board" in
		R58)
			pid_vid=`lsusb | awk '(NF&&$2~/^(002)/&&$6!~/^1d6b/) {print $6}'`
			;;
		R40)
			pid_vid=`lsusb | awk '(NF&&$2~/^(002|004)/&&$6!~/^1d6b/) {print $6}'`
			;;
		*)
			pid_vid=`lsusb | awk '(NF&&$2~/^(002|004)/&&$6!~/^1d6b/) {print $6}'`
			;;
	esac
	#echo "pid_vid=$pid_vid"
	pid=`echo $pid_vid | awk -F: '{print $1}'`
	vid=`echo $pid_vid | awk -F: '{print $2}'`

	if [ -n "$pid" ] && [ -n "$vid" ];	then
		#echo "pid=$pid;vid=$vid"
		SEND_CMD_PIPE_OK_EX $3 "pid=0x$pid;vid=0x$vid"
	else
		echo "host2 no device"
	fi
	sleep 2
done

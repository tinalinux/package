#!/bin/sh

##############################################################################
# \version     1.0.0
# \date        2014年08月28日
# \author      hesimin <hesimin@allwinnertech.com>
# \Descriptions:
#			create the inital version
##############################################################################

source send_cmd_pipe.sh
source script_parser.sh
source sysinfo.sh

kversion=`get_kernel_version`
if [ ${kversion} = "kernel-3.4.39" ]; then
	axppath="/sys/devices/platform/axp*_board/modalias"
	axp_name=`cat $axppath | sed 's/platform:/''/g'`
	SEND_CMD_PIPE_OK_EX $3 "${axp_name}"
elif [ ${kversion} = "kernel-3.10.65" ]; then
	axppath="/sys/class/axp/axp_name"
	axp_name=`cat $axppath`
	SEND_CMD_PIPE_OK_EX $3 "${axp_name}"
else
	SEND_CMD_PIPE_FAIL_EX $3 "NOT PMU"
fi

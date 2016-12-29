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

axppath="/sys/class/axp/axp_name"
if [ -f $axppath ]; then
	axp_name=`cat $axppath`
	SEND_CMD_PIPE_OK_EX $3 "${axp_name}"
else
	SEND_CMD_PIPE_FAIL_EX $3 "NOT PMU"
fi

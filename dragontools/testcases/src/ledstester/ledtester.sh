#!/bin/sh
###############################################################################
# \version     1.0.0
# \date        2012年09月26日
# \author      Martin <zhengjiewen@allwinnertech.com>
# \Descriptions:
#			create the inital version
###############################################################################

source send_cmd_pipe.sh
echo timer > /sys/class/leds/*/trigger

SEND_CMD_PIPE_OK_EX $3 "ALL LEDS flashing"

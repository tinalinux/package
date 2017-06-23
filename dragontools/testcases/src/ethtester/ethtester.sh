#!/bin/sh
##############################################################################
# \version     1.0.0
# \date        2012年05月31日
# \author      James Deng <csjamesdeng@allwinnertech.com>
# \Descriptions:
#			create the inital version

# \version     1.1.0
# \date        2012年09月26日
# \author      Martin <zhengjiewen@allwinnertech.com>
# \Descriptions:
#			add some new features:
#			1.wifi hotpoint ssid and single strongth san
#			2.sort the hotpoint by single strongth quickly
##############################################################################
source send_cmd_pipe.sh
source script_parser.sh

count=0
while true ; do
	ip=$(route -n | awk '($1~/^(0.0.0.0)/&&$2!~/^(0.0.0.0)/) {print $2}' | head -n 1)
	if [ -z "$ip" ]; then
		let count=$count+1
		sleep 3
		continue
	else
		count=0
		result=`ping $ip -c 1 | grep "1 packets transmitted"`
		result=`echo $result | awk '{printf ("%s %s %s", $7, $8, $9) }'`
		echo $result
		# test done
		SEND_CMD_PIPE_OK_EX $3 "$result"
		exit 1
	fi

	if [ $count -gt 20 ]; then
		SEND_CMD_PIPE_FAIL $3
		break
	fi
	sleep 3
done

#!/bin/sh

source send_cmd_pipe.sh
source script_parser_stress.sh

platform=`script_fetch "cpu" "platform"`
workers=`script_fetch "cpu" "workers"`
timeout=`script_fetch "cpu" "timeout"`
outlog=`script_fetch "cpu" "outlogname"`
errlog=`script_fetch "cpu" "errlogname"`


stress --cpu $workers --timeout $timeout 1>/tmp/$outlog 2>/tmp/$errlog
if [ $? -ne 0 ]; then
    echo "cpu stress test fail"
    SEND_CMD_PIPE_FAIL_EX $3 "workers:$workers timeout:$timeout"
else
    echo "cpu stress test success!"
    SEND_CMD_PIPE_OK_EX $3 "workers:$workers timeout:$timeout"
fi

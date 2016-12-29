#!/bin/sh

source send_cmd_pipe.sh
source script_parser_stress.sh

platform=`script_fetch "gpu" "platform"`
width=`script_fetch "gpu" "width"`
height=`script_fetch "gpu" "height"`
timeout=`script_fetch "gpu" "timeout"`
outlog=`script_fetch "gpu" "outlogname"`
errlog=`script_fetch "gpu" "errlogname"`

echo "width=$width"
echo "height=$height"
echo "timeout=$timeout"

opengles_demo --width $width --height $height 1>/tmp/$outlog 2>/tmp/$errlog &
pid=$!
sleep $timeout
ls /proc/$pid > /dev/null
if [ $? -ne 0 ]; then
    echo "gpu stress test fail"
    SEND_CMD_PIPE_FAIL_EX $3 "width:$width height:$height timeout:$timeout"
else
    echo "gpu stress test success!"
    SEND_CMD_PIPE_OK_EX $3 "width:$width height:$height timeout:$timeout"
fi
kill $pid

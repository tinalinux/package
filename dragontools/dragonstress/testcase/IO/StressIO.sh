#!/bin/sh

source send_cmd_pipe.sh
source script_parser_stress.sh

platform=`script_fetch "io" "platform"`
workers=`script_fetch "io" "workers"`
timeout=`script_fetch "io" "timeout"`
test_size=`script_fetch "io" "test_size"`
test_path=`script_fetch "io" "test_path"`
outlog=`script_fetch "io" "outlogname"`
errlog=`script_fetch "io" "errlogname"`

echo "workers=$workers"
echo "timeout=$timeout"
echo "test_size=$test_size"
echo "test_path=$test_path"

if [ -n $test_path ]; then
echo "cd $test_path"
cd $test_path
fi

stress --hdd $workers --hdd-bytes $test_size --timeout $timeout 1>/tmp/$outlog 2>/tmp/$errlog
if [ $? -ne 0 ]; then
    echo "io stress test fail"
    SEND_CMD_PIPE_FAIL_EX $3 "workers:$workers test_size:$test_size timeout:$timeout"
else
    echo "io stress test success!"
    SEND_CMD_PIPE_OK_EX $3 "workers:$workers test_size:$test_size timeout:$timeout"
fi

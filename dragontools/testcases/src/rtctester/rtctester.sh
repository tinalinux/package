#!/bin/sh

source send_cmd_pipe.sh
source script_parser.sh

set_date=`script_fetch "rtc" "set_date"`
if [ -z "$set_date" ]; then
    set_date="2016-04-06 01:56:10"
fi

date -u +"%Y-%m-%d %H:%M:%S" -s "$set_date"

hwclock -w > /dev/null
if [ $? -ne 0 ]; then
    SEND_CMD_PIPE_FAIL $3
    exit 1
fi

hwclock -s > /dev/null
if [ $? -ne 0 ]; then
    SEND_CMD_PIPE_FAIL $3
    exit 1
fi

cur_date=`date -u +"%Y-%m-%d %H:%M:%S"`
cur_date=${cur_date%:*}
set_date=${set_date%:*}
if [ "$cur_date" != "$set_date" ]; then
    SEND_CMD_PIPE_FAIL $3
    exit 1
fi

while true; do
    cur_date=`date -u +"%Y-%m-%d %H:%M:%S"`
    SEND_CMD_PIPE_OK_EX $3 "$cur_date"
    exit 1
done

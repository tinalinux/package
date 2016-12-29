#!/bin/sh

source send_cmd_pipe.sh
source script_parser.sh

platform=`script_fetch "mic" "platform"`
volume=`script_fetch "mic" "volume"`
samplerate=`script_fetch "mic" "samplerate"`
if [ "x$platform" = "xr40" ] ; then
	amixer cset numid=25 1
	sleep 1
	amixer cset numid=39 1
	sleep 1
fi
SEND_CMD_PIPE_OK_EX $3 "mic set finish"

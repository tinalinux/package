#!/bin/sh

source send_cmd_pipe.sh
source script_parser.sh

platform=`script_fetch "headphone" "platform"`
volume=`script_fetch "headphone" "volume"`
samplerate=`script_fetch "headhpone" "samplerate"`
if [ "x$platform" = "xr40" ] ; then
	amixer cset iface=MIXER,name='Headphone volume' $volume
	amixer cset iface=MIXER,name='HPL Mux' 1
	amixer cset iface=MIXER,name='HPR Mux' 1
	amixer cset iface=MIXER,name='Right Output Mixer DACR Switch' 1
	amixer cset iface=MIXER,name='Left Output Mixer DACL Switch' 1
fi

while true
do
	jack=`amixer cget numid=47 | grep "values=on"`
	if [ -z $jack ]; then
		echo "no jack"
	else
		break
	fi
	sleep 1
done

count=0
while true
do
	aplay -fdat /usr/bin/send.pcm
	SEND_CMD_PIPE_OK_EX $3 "playing"
	let count=$count+1
	if [ $count -gt 60 ]; then
		break
	fi
done

SEND_CMD_PIPE_OK_EX $3 "play finish"

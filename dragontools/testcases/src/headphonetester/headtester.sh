#!/bin/sh

source send_cmd_pipe.sh
source script_parser.sh

platform=`script_fetch "headphone" "platform"`
volume=`script_fetch "headphone" "volume"`
playtime=`script_fetch "headphone" "music_playtime"`

if [ "x$platform" = "xr40" ] ; then
	amixer cset iface=MIXER,name='Headphone volume' $volume
	amixer cset iface=MIXER,name='HPL Mux' 1
	amixer cset iface=MIXER,name='HPR Mux' 1
	amixer cset iface=MIXER,name='Right Output Mixer DACR Switch' 1
	amixer cset iface=MIXER,name='Left Output Mixer DACL Switch' 1
	amixer cset iface=MIXER,name='Headphone Switch' 1
elif [ "x$platform" = "xr58" ]; then
	amixer cset iface=MIXER,name='headphone volume' $volume
	amixer cset iface=MIXER,name='AIF1OUT0R Mux' 1
	amixer cset iface=MIXER,name='AIF1OUT0L Mux' 1
	amixer cset iface=MIXER,name='DACR Mixer AIF1DA0R Switch' 1
	amixer cset iface=MIXER,name='DACL Mixer AIF1DA0L Switch' 1
	amixer cset iface=MIXER,name='HP_L Mux' 0
	amixer cset iface=MIXER,name='HP_R Mux' 0
	amixer cset iface=MIXER,name='Headphone Switch' 1
fi

count=0
while true
do
	jack=`amixer cget numid=$(amixer contents | grep "Headphone Switch" | awk 'BEGIN{FS="[,|=]"} {print $2}') | grep "values=on"`
	if [ -z "$jack" ]; then
		echo "no jack"
		let count=$count+1
	else
		break
	fi

	if [ $count -gt 10 ]; then
		SEND_CMD_PIPE_FAIL_EX $3 "no jack"
		exit -1
	fi

	sleep 1
done

count=0
while true
do
	aplay -fdat /usr/bin/send.pcm
	SEND_CMD_PIPE_OK_EX $3 "playing"
	let count=$count+1
	if [ $count -gt $playtime ]; then
		break
	fi
	sleep 1
done

SEND_CMD_PIPE_OK_EX $3 "play finish"

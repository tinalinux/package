#!/bin/sh
# Descriptions:
#	Gets the specified row string
# Parameters:
#	the specified row

#the path to save target file
path_music_list="/usr/share/playdemo_data/music/music_list"
path_music_sum="/usr/share/playdemo_data/music/music_sum"

if [ ! -e $path_music_list ]; then
	exit
fi

if [ ! -e $path_music_sum ]; then
	cat $path_music_list | wc -l >$path_music_sum
fi

if [ $# -ne 1 ]; then
	exit
fi

row_sum=`cat $path_music_sum`
if [ $# -gt $row_sum ] || [ $# -lt 1 ]; then
	exit
fi

sed -n $1p $path_music_list

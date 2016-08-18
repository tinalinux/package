#!/bin/sh
# Descriptions:
#	this script is used in tina v2.0, to scan music files and create music_list for playdmeo to play local music
# Release: 2016/05/10

#the file name to save music list
target="music_list"
target_sum="music_sum"
#the path to save target file
target_path="/usr/share/playdemo_data/music"

#the suffixes of files to scan
suffixes="mp3 ogg wav ape flac wav alac amr m4a m4r \
		  MP3 OGG WAV APE FLAC WAV ALAC AMR M4A M4R"
#the path to scan
scan_paths="/mnt/UDISK /mnt/SDCARD"

get_path() {
	if [ ! -d $target_path ]; then
		echo $target_path is not exit, now to create
		mkdir -p $target_path
	fi

	#get effiective scan path
	for tmp in $scan_paths
	do
		if [ -d $tmp ]; then
			scan_effective_path="$scan_effective_path $tmp"
		fi
	done
}

get_suffixes() {
	for tmp in $suffixes
	do
		if [ -z "$find_suffixes" ]; then
			find_suffixes="-name *.$tmp"
		else
			find_suffixes="$find_suffixes -o -name *.$tmp"
		fi
	done
}

get_path
get_suffixes
find $scan_effective_path $find_suffixes -type f >$target_path/$target 2>/dev/null
cat $target_path/$target | wc -l >$target_path/$target_sum 2>/dev/null

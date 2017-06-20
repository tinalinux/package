#!/bin/sh

key="/demo/demo-sh/command"
val=$(mjson_fetch ${key})

echo "key: ${key}"
echo "val: ${val}"

echo

target=$(get_target)
platform=$(get_platform)
boot_media=$(get_boot_media)

echo "target: ${target}"
echo "platform: ${platform}"
echo "boot_media: ${boot_media}"

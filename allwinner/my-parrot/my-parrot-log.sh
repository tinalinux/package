#!/bin/sh

cd /usr/bin/speech && ./bootloader-awr16.sh start &>/dev/ttyS0
my-parrot &>/dev/ttyS0

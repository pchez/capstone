#!/bin/sh

sed 's/Notification handle = 0x0011 value: //' < motion_raw.txt > motion_out_raw.txt
sed -i '1d' motion_out_raw.txt
#sed -i "s/Characteristic value was written successfully//"  motion_raw_test.txt

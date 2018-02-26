#!/bin/sh

sed -i 's/Notification handle = 0x0011 value: //'  motion_raw_test.txt
sed -i '1d' motion_raw_test.txt
#sed -i "s/Characteristic value was written successfully//"  motion_raw_test.txt

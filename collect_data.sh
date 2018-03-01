#!/bin/sh

trap 'echo "Caught SIGUSR1"' USR1
echo "Sleeping"
while :
do
    sleep infinity &
    wait
done

echo Collecting data...
gatttool -b C0:85:40:31:4D:48 -t random --char-write-req --handle=0x0012 --value=0100 --listen > motion_raw.txt &
PID=$!
echo PID of gatttool: $PID
sleep 5
echo End gatttool: $PID
kill -INT $PID

sed 's/Notification handle = 0x0011 value: //' < motion_raw.txt > motion_preprocessed.txt
sed -i '1d' motion_preprocessed.txt
python convert_data.py motion_preprocessed.txt motion_converted.txt 1
#sed -i "s/Characteristic value was written successfully//"  motion_raw_test.txt

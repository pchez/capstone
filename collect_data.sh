#!/bin/sh

trap 'echo "Caught SIGUSR1"' USR1
trap 'trap - TERM && kill -- -$$' INT TERM EXIT

alias errcho='>&2 echo'

collectData()
{
#    echo Collecting data...
#    gatttool -b C0:85:40:31:4D:48 -t random --char-write-req --handle=0x0012 --value=0100 --listen > motion_raw.txt &
#    PID=$!
#    echo PID of gatttool: $PID
#    sleep 5
#    echo End gatttool: $PID
#    kill -INT $PID
#
#    sed 's/Notification handle = 0x0011 value: //' < motion_raw.txt > motion_preprocessed.txt
#    sed -i '1d' motion_preprocessed.txt
#    python convert_data.py motion_preprocessed.txt motion_converted.txt 1
#    #sed -i "s/Characteristic value was written successfully//"  motion_raw_test.txt
    
    errcho "start data collection\n"
    sleep 1
    kill -USR1 $1

}

errcho "collect_data: sleeping\n"
while :
do
    sleep infinity &
    SLEEP_PID=$!
    wait

    collectData
done



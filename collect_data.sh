#!/bin/sh

alias errcho='>&2 echo'
finished=0

#trap 'kill $SLEEP_INF_PID' INT TERM EXIT
#trap 'collectData; errcho "Caught SIGUSR1"' USR1
trap collectData USR1
trap run USR2
trap 'trap - INT && finished=1' INT TERM EXIT
#trap "trap - INT && kill -- -$(ps -o pgid=$PID | grep -o [0-9]*) && kill $$" INT TERM EXIT
LABEL=0

collectData()
{
    errcho "collect_data: Collecting data for training..."

    gatttool -b C0:85:40:31:4D:48 -t random --char-write-req --handle=0x0012 --value=0100 --listen > motion_raw.txt &
    GATT_PID=$!
    errcho PID of gatttool: $GATT_PID
    sleep 5
    kill -TERM $GATT_PID
    SLEEP_COLLECT_PID=$!

    errcho "collect_data: finished, converting...\n"    
    sed 's/Notification handle = 0x0011 value: //' < motion_raw.txt > motion_preprocessed.txt
    sed -i '1d' motion_preprocessed.txt
    LABEL=`expr $LABEL + 1`
    python convert_data.py motion_preprocessed.txt motion_converted.txt $LABEL
    #sed -i "s/Characteristic value was written successfully//"  motion_raw_test.txt
    sleep 1
    kill -USR1 $PARENT_PID
    
}

run()
{
    errcho "collect_data: Collecting data for run-time test..."
    sleep 1
    kill -USR2 $PARENT_PID
}

PARENT_PID=$1
errcho "collect_data parent pid:" $PARENT_PID

while [ $finished -ne 1 ]
do
   # errcho "collect_data: sleeping\n"
    sleep 1 & SLEEP_INF_PID=$!
    wait
done

#kill $(ps -s $$ -o pid=)
exit 0

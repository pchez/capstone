#!/bin/sh
#
# Usage sensor_sample.sh -t <TIME IN SECONDS> -f <OUTPUT_FILE_NAME> -p <PARENT_PID>
#
# Enter Device MAC Address below 
#
#

alias errcho='>&2 echo'
finished=0

trap collectData USR1
trap 'trap - INT && finished=1' INT TERM EXIT

LABEL=0

collectData()
{
    gatttool -b C0:85:40:31:4D:48 -t random --char-write-req --handle=0x0012 --value=0100 --listen > sensor_data_stream.dat &
    sleep $TIME

    tail -n 50 sensor_data_stream.dat > motion_data.dat
    kill -USR1 $PARENT_PID
    
    pkill gatttool    
    
}



while getopts t:f:p: option
do
	 case "${option}"
		  in
		   t) TIME=${OPTARG};;
		   f) FILE=${OPTARG};;
		   p) PARENT_PID=${OPTARG};;
         esac
done


while [ $finished -ne 1 ]
do
    sleep 1 & SLEEP_INF_PID=$!
    wait
done

exit 0

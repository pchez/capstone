#!/bin/sh
#
# Usage sensor_sample.sh -t <TIME IN SECONDS> -f <OUTPUT_FILE_NAME>
#
# Enter Device MAC Address below 
#
#
#trap 'echo trying to exit && pkill gatttool && exit 0' INT

while getopts t:f: option
do
	 case "${option}"
		  in
		   t) TIME=${OPTARG};;
		   f) FILE=${OPTARG};;
         esac
done

if [ "$TIME" -lt "2" ]
then
	  echo "Error time less than 2 seconds"
	  exit
fi

if [ "$TIME" -gt "10" ]
then
	  echo "Error time greater than 10 seconds"
	  exit

fi

gatttool -b C0:85:40:31:4D:48 -t random --char-write-req --handle=0x0012 --value=0100 --listen > $FILE  &

sleep $TIME

pkill gatttool


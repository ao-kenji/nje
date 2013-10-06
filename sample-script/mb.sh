#!/bin/sh
#
# mb.sh --- sample Bourne shell script for NJE-10[56]
#

DEVICE=/dev/tty00
NEWS_FILE="./news.sjis"

while :
do
	if [ -e ${NEWS_FILE} ]; then
		cat ${NEWS_FILE} | while read line
		do
			./nje -f ${DEVICE} '~AA~' ${line}
			sleep 5
		done
	fi
done

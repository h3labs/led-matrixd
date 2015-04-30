#!/bin/bash
#exit
beacon="/home/pi/signcfg/shopisopen.beacon"

if [ -e  "$beacon" ]
then
#	echo "beacon is present"
	ssh hlaborat@h3laboratories.com << EOF
#date
touch shopisopen.beacon
EOF
	ttytter -ssl -status="H3 Labs is OPEN at `date +'%I:%M %P'` #makerspace #hackerspace #elpaso"
else
#	echo "beacon is absent"
	ssh hlaborat@h3laboratories.com << EOF
#date
rm shopisopen.beacon
EOF
	ttytter -ssl -status="H3 Labs is CLOSED as of `date +'%I:%M %P'` #elpaso"
fi


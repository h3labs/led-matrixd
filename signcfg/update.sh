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
else
#	echo "beacon is absent"
	ssh hlaborat@h3laboratories.com << EOF
#date
rm shopisopen.beacon
EOF
fi


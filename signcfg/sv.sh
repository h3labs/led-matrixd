#!/bin/bash

beacon="/home/pi/signcfg/shopisopen.beacon"

if [ -e  "$beacon" ]
then
	echo "beacon is present"
sshpass -p '!Tritium36288001' ssh -o StrictHostKeyChecking=no h3labs@h3laboratories.com << EOF
date
touch shopisopen.beacon
EOF
else
	echo "beacon is absent"
sshpass -p '!Tritium36288001' ssh -o StrictHostKeyChecking=no h3labs@h3laboratories.com << EOF
date
rm shopisopen.beacon
EOF
fi


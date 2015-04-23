#!/bin/bash
#exit
beacon="$1"

if [ -e  "$beacon" ]; then
	#	echo "beacon is present"
	#	ssh hlaborat@h3laboratories.com << EOF
	#date
	#touch shopisopen.beacon
	#EOF
	echo "updated $beacon"
else
	#	echo "beacon is absent"
	#	ssh hlaborat@h3laboratories.com << EOF
	#date
	#rm shopisopen.beacon
	#EOF
	echo "updated $beacon"
fi


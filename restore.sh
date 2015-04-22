#!/bin/bash
#exit
beacon="$1"

echo "The beacon file is $beacon with $# parameters"
if [ ! -e  "$beacon" ]
then
	echo -n "hey=there" > "$beacon"
fi


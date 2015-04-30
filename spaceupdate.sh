#!/bin/bash
#exit
beacon="$1"

open_tags="#makerspace #hackerspace #elpaso"
closed_tags="#elpaso"

if [ -e  "$beacon" ]
then
	c=`cat $beacon | xargs`
#	echo "beacon is present"
	ssh hlaborat@h3laboratories.com << EOF
	php spaceapi_setup.php --state='${c}'
EOF
	echo ${c}
	d=`date +'%I:%M %P' | xargs`
	if [[ $c == *"&state=open"* ]]
	then
		s="OPEN"
		r="${s} at"
		t="$open_tags"
	else
		s="CLOSED"
		r="${s} as of"
		t="$closed_tags"
	fi

	if [[ $c == *"&msg="* ]]
	then
		m=`echo -n ${c} | xargs | awk 'BEGIN {FS="&msg="} {print $2}'`
		m=`urlencode -d "${m}"`
		m=": ${m}"
	else
		m=''
	fi

	ttytter -ssl -status="H3 Labs is ${r} ${d}${m} ${t}"
fi


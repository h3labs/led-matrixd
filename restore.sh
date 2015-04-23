#!/bin/bash
#exit
beacon="$1"

urlencode() 
{
	# urlencode <string>
	local length="${#1}"
	for (( i = 0; i < length; i++ )); do
		local c="${1:i:1}"
		case $c in
			[a-zA-Z0-9.~_-]) printf "$c" ;;
			*) printf '%%%02X' "'$c"
		esac
	done
}

urldecode() 
{
	# urldecode <string>

	local url_encoded="${1//+/ }"
	printf '%b' "${url_encoded//%/\x}"
}

echo "The beacon file is $beacon with $# parameters"
if [ ! -e  "$beacon" ]
then
	t=$(date -u)
	u="neon"
	msg="booting up!"
	out=$(urlencode "user=$u&time=$t&msg=$msg")
	echo "restoring with \"$out\""
	echo -n "$out" > "$beacon"
fi


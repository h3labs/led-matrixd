#!/bin/bash
#(while :; do date "+ %a%n%b%d%n%R" ; sleep 0.2 ; done) | ./text-example -f fonts/6x9.bdf -y 2 -x 1 -C$((RANDOM % 255)),$((RANDOM % 255)),$((RANDOM % 255))
#(while :; do ; done) | ./led-matrix -t 5 -D 1 -m -1 $(echo `ls -1 ../signcfg/sprites/|grep -v mf0|sort -R|head -1` )

while :;
do
	F=`ls -1 ../signcfg/sprites/|sort -R|head -1`
#	echo $F
	./led-matrix -t 5 -D 1 -m -1 ../signcfg/sprites/${F}
done

#ls -1 ../signcfg/sprites/|grep -v mf0|sort -R|head -1

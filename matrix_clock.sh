#!/bin/bash
(while :; do date "+ %a%n%b%d%n%R" ; sleep 1 ; done) | ./text-example -f fonts/6x9.bdf -y 2 -x 1 -C255,204,0


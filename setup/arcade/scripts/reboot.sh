#!/bin/sh

function cmd () {

	echo "root"
	sleep 2s
	echo "mcu"
	sleep 2s
	echo "reboot"

}


test -z "$1" && exit 0

for n in $*; do
	cmd | nc -w2 -t 23.23.23.$n 23
done


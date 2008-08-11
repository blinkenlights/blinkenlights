#!/bin/sh

count=0

while :; do
	sleep 7s

	foo=$(ps aux | grep mcud | grep -v grep | grep -v watchdog)
	if test -z "$foo"; then
		/etc/init.d/mcud start
		count=$(($count + 1))
	else
		count=0
	fi

	if test $count -gt 2; then
		reboot
	fi
done

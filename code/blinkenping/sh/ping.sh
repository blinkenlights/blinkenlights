#!/bin/sh

while [ TRUE ]; do
	for x in $(cat hostlist); do
		./ping_one.sh $x $x.result &
	done

	sleep 5s;
done


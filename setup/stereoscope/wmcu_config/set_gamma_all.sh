#!/bin/sh

source all_ips.inc

if [ ! -f "$1" ]; then
	echo "Usage: $0 <filename>"
	exit 1
fi

for ip in $IPS; do
	./wdim_configurator.py --host $ip --lamp-mac 0xffff --set-gamma $1
done


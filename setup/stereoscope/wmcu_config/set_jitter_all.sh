#!/bin/sh

source all_ips.inc

if [ -z "$1" ]; then
	echo "Usage: $0 <value>"
	exit 1
fi

for ip in $IPS; do
	./wdim_configurator.py --host $ip --lamp-mac 0xffff --set-dimmer-jitter $1
done


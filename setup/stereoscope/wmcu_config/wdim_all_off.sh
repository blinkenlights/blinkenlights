#!/bin/sh

source all_ips.inc

for ip in $IPS; do
	./wdim_configurator.py --host $ip --lamp-mac 0xffff --dimmer-off 1
done


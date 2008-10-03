#!/bin/sh

source all_ips.inc

for ip in $IPS; do
	./wmcu_configurator.py --host $ip --get-statistics
done


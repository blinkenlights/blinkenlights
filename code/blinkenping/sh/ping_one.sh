#!/bin/sh

ping -c1 -W2 -q $1 > /dev/null 2>&1
echo $? > $2


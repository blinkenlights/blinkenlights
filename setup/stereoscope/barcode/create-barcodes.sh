#!/bin/sh

barcode -e "128b" -u mm -p 210x297mm -t 3x6+10+5-5-23 -m 5 -o barcode.ps -i wdim-macs.txt

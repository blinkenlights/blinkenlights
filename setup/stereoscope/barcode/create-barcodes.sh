#!/bin/sh
#
# Create 3x6 (18) Code128b barcodes per page on DIN A4 size
#
# This tool requires GNU Barcode to be installed

barcode -e "128b" -u mm -p 210x297mm -t 3x6+10+5-5-23 -m 5 -o barcode.ps -i wdim-macs.txt

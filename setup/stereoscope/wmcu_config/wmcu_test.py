#!/usr/bin/python

import getopt
import socket
import sys
import struct
import time

params		= [0, 0, 0, 0, 0, 0, 0, 0];

host 		= "169.254.17.34"
port		= 2323

RF_CMD_SET_VALUES	= 0
DEBUG_SEND_RAW		= 0xff
MCUCTRL_MAGIC 		= 0x23542667
val = 0

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect((host, port))

while 1:
	val = val & 0xf
	val = val + 1
	val &= 0xf
	val |= val << 4

	print (" val %02x" % val)

	packet = struct.pack("!IIIIIII26I", MCUCTRL_MAGIC, DEBUG_SEND_RAW, 0, 0,
			RF_CMD_SET_VALUES, 0xffff, 0,
			val, val, val, val, val, val, val, val,
			val, val, val, val, val, val, val, val,
			val, val, val, val, val, val, val, val,
			val, val);

	s.send(packet)
	time.sleep(0.5)

s.close


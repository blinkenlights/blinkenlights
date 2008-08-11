#!/usr/bin/python

import getopt
import socket
import sys
import struct

def usage():
	print("Blinkenlights Wireless MCU setup tool\n")
	print("Usage: %s [--help] [--host <ip>] [--port <port>] [--set key=value]" % sys.argv[0])
	print("\t--help			this screen")
	print("\t--host <ip>		the IP address to connect to")
	print("\t--port <port>		the port to use, defaults to 2323")
	print("\t--set-line <line>	configure the WMCU to listen to line #<line>")
	print("\t--set-lamp-id <mac>=<id>	")
	sys.exit(1)

action = -1
line = -1
host = ""
port = 2323

SET_LINE = 0
SET_LAMPID = 1
MCUCTRL_MAGIC = 0x23542667

try:
	opts, args = getopt.getopt(sys.argv[1:], "hh:p:s:", ["help", "host=", "port=", "setline="])
except getopt.GetoptError, err:
	print str(err)
	usage()

for o, a in opts:
	if o == "--help":
		usage()
	if o == "--host":
		host = a
	if o == "--port":
		port = int(a)
	if o == "--setline":
		action = SET_LINE
		line = int(a)

print("action %d host %s" % (action, host))

if action == -1:
	print("need an action to perform.\n")
	usage()

if action == SET_LINE:
	packet = struct.pack("!III", MCUCTRL_MAGIC, 0, line)
elif action == SET_LAMPID:
	packet = struct.pack("!III", MCUCTRL_MAGIC, 1, )

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect((host, port))
s.send(packet)
s.close


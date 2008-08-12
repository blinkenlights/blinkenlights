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
	print("\t--set-lamp-id <id>	")
	sys.exit(1)

action = -1
line = -1
lampmac = 0

host = ""
port = 2323

SET_LINE = 0
SET_LAMPID = 1
MCUCTRL_MAGIC = 0x23542667

try:
	opts, args = getopt.getopt(sys.argv[1:], "hh:p:s:s:l:", ["help", "host=", "port=", "set-line=", "set-lamp-id=", "lamp-mac="])
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
	if o == "--lamp-mac":
		lampid = int(a)
	if o == "--set-line":
		action = SET_LINE
		line = int(a)
	if o == "--set-lamp-id":
		action = SET_LAMPID
		lampid = int(a)

if action == -1:
	print("need an action to perform.\n")
	usage()

if action == SET_LINE:
	packet = struct.pack("!IIII", MCUCTRL_MAGIC, 0, line, 0)
elif action == SET_LAMPID:
	if lampmac == 0:
		usage()

	packet = struct.pack("!IIII", MCUCTRL_MAGIC, 1, lampid, lampmac)

if host == "":
	usage()

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect((host, port))
s.send(packet)
s.close


#!/usr/bin/python

import getopt
import socket
import sys
import struct

def read_list_file(filename, intbase):
	f = open(filename, 'r')
	if (f is None):
		return None;

	gamma = []
	s = f.read()
	for t in s.split():
		gamma.append(int(t, intbase))

	return gamma


def usage():
	print("Blinkenlights Wireless MCU setup tool\n")
	print("Usage: %s [--help] [options]" % sys.argv[0])
	print("\t--help					this screen")
	print("\t--host <ip>				the IP address to connect to")
	print("\t--port <port>				the port to use, defaults to 2323")
	print("\t--set-mcu-id <id>			configure the WMCU's ID")
	print("\t--set-assigned-lamps <filename>		set the lamps assigned to an WMCU");
	sys.exit(1)

action = -1
mcu_id = -1

packet		= ""
packet2		= ""

host 		= ""
port		= 2323

SET_MCUID		= 0
SET_ASSIGNED_LAMPS	= 5
DEBUG_SEND_RAW		= 0xff

MCUCTRL_MAGIC 	= 0x23542667

try:
	opts, args = getopt.getopt(sys.argv[1:],
		"hh:p:s:s",
		["help", "host=", "port=", "set-mcu-id=", "set-assigned-lamps="])

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
	if o == "--set-mcu-id":
		action = SET_MCUID
		mcu_id = int(a)
	if o == "--set-assigned-lamps":
		action = SET_ASSIGNED_LAMPS
		lamp_list_file = a

if action == -1:
	print("need an action to perform.\n")
	usage()

if action == SET_MCUID:
	packet = struct.pack("!IIII8I", MCUCTRL_MAGIC, action, 0, mcu_id)

elif action == SET_ASSIGNED_LAMPS:
	lamps = read_list_file(lamp_list_file, 16)
	print lamps

	packet = struct.pack("!IIII16I", MCUCTRL_MAGIC, action, 0, 0,
			lamps[0],  lamps[1],  lamps[2],  lamps[3],
			lamps[4],  lamps[5],  lamps[6],  lamps[7],
			lamps[8],  lamps[9],  lamps[10], lamps[11],
			lamps[12], lamps[13], lamps[14], lamps[15])

if host == "":
	usage()

while len(packet) < 64:
	pad = struct.pack("x")
	packet += pad

while len(packet2) < 64:
	pad = struct.pack("x")
	packet2 += pad

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect((host, port))
s.send(packet)
if (packet2):
	s.send(packet2)

s.close


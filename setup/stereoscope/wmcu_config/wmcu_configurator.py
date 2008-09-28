#!/usr/bin/python

import getopt
import socket
import sys
import struct

def read_lamp_map(filename):
	file = open(filename, 'r')
	if (file is None):
		return None;

	list = []
	count = 0

	for line in file:
		if line[0] == '#':
			continue

		a = line.split()
		if len(a) < 4:
			continue

		list.append(int(a[0], 16))
		list.append(int(a[1], 10))
		list.append(int(a[2], 10))
		list.append(int(a[3], 10))
		count += 1

	print "%d lamps read from file" % (count)
	return  list


def usage():
	print("Blinkenlights Wireless MCU setup tool\n")
	print("Usage: %s [--help] [options]" % sys.argv[0])
	print("\t--help					this screen")
	print("\t--host <ip>				the IP address to connect to")
	print("\t--port <port>				the port to use, defaults to 2323")
	print("\t--set-mcu-id <id>			configure the WMCU's ID")
	print("\t--set-rf-delay <delay>			configure the WMCU's RF delay time in ms")
	print("\t--set-assigned-lamps <filename>		set the lamps assigned to an WMCU");
	print("\t--set-rf-power <val>			set the WMCU RF power");
	print("\t--set-jam-density <val>		set the WMCU jam density (time between pkts in ms)");
	print("\t--reset-device				reset the WMCU");
	sys.exit(1)

action		= -1
mcu_id		= -1
packet		= ""
host 		= ""
port		= 2323

SET_MCUID		= 0
SET_ASSIGNED_LAMPS	= 5
SET_RF_DELAY		= 7
SET_RF_POWER		= 10
SET_JAM_DENSITY		= 11
RESET_DEVICE		= 13

MCUCTRL_MAGIC 	= 0x23542667

try:
	opts, args = getopt.getopt(sys.argv[1:],
		"hh:p:s:s:s:s:s:r",
		["help", "host=", "port=", "set-mcu-id=", "set-rf-delay=", "set-assigned-lamps=",
		 "set-rf-power=", "set-jam-density=", "reset-device" ])

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
	if o == "--set-rf-delay":
		action = SET_RF_DELAY
		delay = int(a)
	if o == "--set-assigned-lamps":
		action = SET_ASSIGNED_LAMPS
		lamp_list_file = a
	if o == "--set-rf-power":
		action = SET_RF_POWER
		power = int(a)
	if o == "--set-jam-density":
		action = SET_JAM_DENSITY
		density = int(a)
	if o == "--reset-device":
		action = RESET_DEVICE

if action == -1:
	print("need an action to perform.\n")
	usage()

if action == SET_MCUID:
	packet = struct.pack("!IIII", MCUCTRL_MAGIC, action, 0, mcu_id)

if action == SET_RF_DELAY:
	packet = struct.pack("!IIII", MCUCTRL_MAGIC, action, 0, delay)

if action == SET_RF_POWER:
	packet = struct.pack("!IIII", MCUCTRL_MAGIC, action, 0, power)

if action == SET_JAM_DENSITY:
	packet = struct.pack("!IIII", MCUCTRL_MAGIC, action, 0, density)

if action == RESET_DEVICE:
	packet = struct.pack("!IIII", MCUCTRL_MAGIC, action, 0, 0)

elif action == SET_ASSIGNED_LAMPS:
	lamps = read_lamp_map(lamp_list_file)

	packet = struct.pack("!IIII", MCUCTRL_MAGIC, action, 0, 0);

	for v in lamps:
		p = struct.pack("!I", v)
		packet += p

if host == "":
	usage()

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect((host, port))
s.send(packet)
s.close


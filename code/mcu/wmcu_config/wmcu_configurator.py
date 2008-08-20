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
	print("\t--set-lamp-id <id>			sets the id of an lamp, requires --lamp-mac")
	print("\t--set-gamma <filename>			sets the gamma curve for a lamp");
	print("\t--set-dimmer-jitter <jitter>		sets the dimmer jitter for a lamp (in us)");
	print("\t--write-config				makes the lamp write its config (gamma and jitter)");
	print("\t--set-assigned-lamps <filename>		set the lamps assigned to an WMCU");
	print("\t--lamp-mac <id>				specify the lamp MAC address to use for other commands (0xffff for broadcast)")
	print("\t--enter-update-mode			makes an MCU enter its update mode. USE WITH CARE!");
	sys.exit(1)

action = -1
mcu_id = -1
lampmac = 0

params		= [0, 0, 0, 0, 0, 0, 0, 0];
packet		= ""
packet2		= ""

host 		= ""
port		= 2323

SET_MCUID		= 0
SET_LAMPID 		= 1
SET_GAMMA 		= 2
WRITE_CONFIG 		= 3
SET_JITTER		= 4
SET_ASSIGNED_LAMPS	= 5
ENTER_UPDATE_MODE	= 0x3f
DEBUG_SEND_RAW		= 0xff

MCUCTRL_MAGIC 	= 0x23542667

try:
	opts, args = getopt.getopt(sys.argv[1:],
		"hh:p:s:s:s:ws:l:s:e", 
		["help", "host=", "port=", "set-mcu-id=", "set-lamp-id=", 
		 "set-gamma=", "write-config", "set-dimmer-jitter=", "lamp-mac=",
		 "set-assigned-lamps=", "enter-update-mode"])

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
		lampmac = int(a, 16)
	if o == "--set-mcu-id":
		action = SET_MCUID
		mcu_id = int(a)
	if o == "--set-lamp-id":
		action = SET_LAMPID
		lampid = int(a)
	if o == "--set-gamma":
		action = SET_GAMMA
		gamma_filename = a;
	if o == "--write-config":
		action = WRITE_CONFIG
	if o == "--set-dimmer-jitter":
		action = SET_JITTER
		jitter = int(a)
	if o == "--set-assigned-lamps":
		action = SET_ASSIGNED_LAMPS
		lamp_list_file = a
	if o == "--enter-update-mode":
		action = ENTER_UPDATE_MODE

if action == -1:
	print("need an action to perform.\n")
	usage()

if action == SET_MCUID:
	packet = struct.pack("!IIII8I", MCUCTRL_MAGIC, action, 0, mcu_id,
			params[0], params[1], params[2], params[3],
			params[4], params[5], params[6], params[7])

elif action == SET_LAMPID:
	if lampmac == 0:
		usage()

	packet = struct.pack("!IIII8I", MCUCTRL_MAGIC, action, lampmac, lampid, 
			params[0], params[1], params[2], params[3],
			params[4], params[5], params[6], params[7])

elif action == SET_GAMMA:
	if lampmac == 0:
		usage()

	gamma = read_list_file(gamma_filename, 10)
	packet = struct.pack("!IIII8I", MCUCTRL_MAGIC, action, lampmac, 0,
			gamma[0], gamma[1], gamma[2], gamma[3],
			gamma[4], gamma[5], gamma[6], gamma[7])

	packet2 = struct.pack("!IIII8I", MCUCTRL_MAGIC, action, lampmac, 1,
			gamma[8], gamma[9], gamma[10], gamma[11],
			gamma[12], gamma[13], gamma[14], gamma[15])

elif action == WRITE_CONFIG:
	if lampmac == 0:
		usage()
	
	packet = struct.pack("!IIII8I", MCUCTRL_MAGIC, action, lampmac, 0,
			params[0], params[1], params[2], params[3],
			params[4], params[5], params[6], params[7])

elif action == SET_JITTER:
	if lampmac == 0:
		usage()
	
	packet = struct.pack("!IIII8I", MCUCTRL_MAGIC, action, lampmac, jitter,
			params[0], params[1], params[2], params[3],
			params[4], params[5], params[6], params[7])

elif action == SET_ASSIGNED_LAMPS:
	lamps = read_list_file(lamp_list_file, 16)
	print lamps

	packet = struct.pack("!IIII16I", MCUCTRL_MAGIC, action, lampmac, 0,
			lamps[0],  lamps[1],  lamps[2],  lamps[3],
			lamps[4],  lamps[5],  lamps[6],  lamps[7],
			lamps[8],  lamps[9],  lamps[10], lamps[11],
			lamps[12], lamps[13], lamps[14], lamps[15])

elif action == ENTER_UPDATE_MODE:
	if lampmac == 0:
		usage()

	print("sending dimmer to update mode. cross your fingers.")
	packet = struct.pack("!IIIIIII4I", MCUCTRL_MAGIC, DEBUG_SEND_RAW, 0, 0,
        	        0x3f, lampmac, 0,
			0xde, 0xad, 0xbe, 0xef)


if host == "":
	usage()

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect((host, port))
s.send(packet)
if (packet2):
	s.send(packet2)

s.close


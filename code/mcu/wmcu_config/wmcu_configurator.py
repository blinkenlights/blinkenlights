#!/usr/bin/python

import getopt
import socket
import sys
import struct

def read_list_file(filename):
	f = open(filename, 'r')
	if (f is None):
		return None;

	gamma = []
	s = f.read()
	for t in s.split():
		gamma.append(int(t))

	return gamma


def usage():
	print("Blinkenlights Wireless MCU setup tool\n")
	print("Usage: %s [--help] [options]" % sys.argv[0])
	print("\t--help				this screen")
	print("\t--host <ip>			the IP address to connect to")
	print("\t--port <port>			the port to use, defaults to 2323")
	print("\t--set-mcu-id <id>		configure the WMCU's ID")
	print("\t--set-lamp-id <id>		sets the id of an lamp, requires --lamp-mac")
	print("\t--set-gamma <g1>,<g2>,...<g8>	sets the gamma curve for a lamp");
	print("\t--write-gamma			makes the lamp write its gamma curve");
	print("\t--lamp-mac <id>		specify the lamp MAC address to use for other commands (0xffff for broadcast)")
	sys.exit(1)

action = -1
line = -1
lampmac = 0

params		= [0, 0, 0, 0, 0, 0, 0, 0];
packet		= ""
packet2		= ""

host 		= ""
port		= 2323

SET_MCUID	= 0
SET_LAMPID 	= 1
SET_GAMMA 	= 2
WRITE_GAMMA 	= 3
MCUCTRL_MAGIC 	= 0x23542667

try:
	opts, args = getopt.getopt(sys.argv[1:],
		"hh:p:s:s:s:wl:", 
		["help", "host=", "port=", "set-mcu-id=", "set-lamp-id=", "set-gamma=", "write-gamma", "lamp-mac="])

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
		action = SET_LINE
		line = int(a)
	if o == "--set-lamp-id":
		action = SET_LAMPID
		lampid = int(a)
	if o == "--set-gamma":
		action = SET_GAMMA
		gamma_filename = a;
	if o == "--write-gamma":
		action = WRITE_GAMMA

if action == -1:
	print("need an action to perform.\n")
	usage()

if action == SET_LINE:
	packet = struct.pack("!IIII8I", MCUCTRL_MAGIC, 0, 0, line,
			params[0], params[1], params[2], params[3],
			params[4], params[5], params[6], params[7])

elif action == SET_LAMPID:
	if lampmac == 0:
		usage()

	packet = struct.pack("!IIII8I", MCUCTRL_MAGIC, 1, lampmac, lampid, 
			params[0], params[1], params[2], params[3],
			params[4], params[5], params[6], params[7])

elif action == SET_GAMMA:
	if lampmac == 0:
		usage()

	gamma = read_list_file(gamma_filename)
	packet = struct.pack("!IIII8I", MCUCTRL_MAGIC, 2, lampmac, 0,
			gamma[0], gamma[1], gamma[2], gamma[3],
			gamma[4], gamma[5], gamma[6], gamma[7])

	packet2 = struct.pack("!IIII8I", MCUCTRL_MAGIC, 2, lampmac, 1,
			gamma[8], gamma[9], gamma[10], gamma[11],
			gamma[12], gamma[13], gamma[14], gamma[15])


elif action == WRITE_GAMMA:
	if lampmac == 0:
		usage()
	
	packet = struct.pack("!IIII8I", MCUCTRL_MAGIC, 3, lampmac, 0,
			params[0], params[1], params[2], params[3],
			params[4], params[5], params[6], params[7])

if host == "":
	usage()

s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
s.connect((host, port))
s.send(packet)
if (packet2):
	s.send(packet2)

s.close


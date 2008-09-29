#!/usr/bin/python2.5

from socket import *
import struct

# Set the socket parameters
host = ""
port = 23232
buf = 1024
addr = (host,port)

# Create socket and bind to address
UDPSock = socket(AF_INET,SOCK_DGRAM)
UDPSock.bind(addr)

# Receive messages
while 1:
	(data, addr) = UDPSock.recvfrom(buf)
	if not data:
		continue
	
	(magic, command, mac, value) = struct.unpack_from("!IIII", data)

	if magic == 0xfeedbacd:
		(packet_count, emi_pulses, pings_lost, fw_version, ticks) \
			= struct.unpack_from("!IIII", data, 4*4)
		
		print "dimmer response: mac=%04x packet_count=%d emi_pulses=%d pings_lost=%d fw_version=%08x uptime=%03d:%02d:%02d" \
			% (mac, packet_count, emi_pulses, pings_lost, fw_version, ticks / 3600, (ticks / 60) % 60, ticks % 60)
	if magic == 0xfeedbacc:
		(wmcu_id, b_rec_total, rf_sent_broadcast, rf_sent_unicast, rf_rec, jam_density, power_level, n_lamps, version, ticks) \
			= struct.unpack_from("!IIIIIIIIII", data, 4*4)
		print "wmcu respone: id=%d b_rec_total=%d rf_sent_broadcast=%d rf_sent_unicast=%d rf_rec=%d jam_density=%d power_level=%d n_lamps=%d version=%08x uptime=%03d:%02d:%02d" \
			% (wmcu_id, b_rec_total, rf_sent_broadcast, rf_sent_unicast, rf_rec, jam_density,\
			power_level, n_lamps, version, ticks / 3600, (ticks / 60) % 60, ticks % 60)

UDPSock.close()


#!/usr/bin/python

for screen in [ 1, 2, 3, 4 ]:
	if (screen == 1):
		floors = range(4, 10)
	elif (screen == 2):
		floors = range(12, 19)
	elif (screen == 3):
		floors = range(4, 12)
	elif (screen == 4):
		floors = range(14, 25)
	
	for floor in floors:
		print "host wmcu-%d-%d {" % (screen, floor)
        	print "\thardware ethernet 00:bd:33:06:%02x:%02x;" % (screen, floor)
	        print "\tfixed-address 192.168.%d.%d;" % (screen, floor)
	        #print "\tfixed-address wmcu-%d-%d;" % (screen, floor)
	        print "\toption subnet-mask 255.255.255.0;"
	        print "\toption broadcast-address 192.168.%d.255;" % screen
		print "}"
		print


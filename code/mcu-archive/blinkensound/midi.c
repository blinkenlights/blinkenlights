/*
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>

#include "values.h"
#include "midi.h"

int midi_control = 1;
int midi_map[0x7f][0x10];

int midi_open (void) {
	int fd;
	
	fd = open (MIDIDEV, O_RDONLY | O_NONBLOCK);
	if (fd < 0) {
		perror ("open "MIDIDEV);
		return -1;
	}
	
	return fd;
}

void midi_install_default_map (void) {
	int x, c;
	
	for (x=0; x<WIDTH; x++)
		for (c=0; c<0x10; c++)
			midi_map[KEYSTART_DIRECT+x][c] = x;
}

int midi_parse (unsigned char *buf, int size) {
	int pos = 0, ret = 0, i;
	static int command = 0x0, channel = 0x0, sysex = 0;
	static int key=-1, velocity=-1;
	static int h=-1, l=-1;
	static char chromatic[] = { 0, 0, 1, 1, 2, 3, 3, 4, 4, 5, 5, 6, 6, 
				    7, 7, 8, 8, 9, 10, 10, 11, 11, 12, 12, 13, 13,
				    14, 14, 15, 15 };

	while (pos < size) {
		
		printf ("MIDI IN (%d bytes), command 0x%02x: ", size, command);
		for (i=0; i < size; i++) printf ("%02x ", buf[i]);
		printf ("\n");
		

		if (buf[pos] & 0x80) {
		
			if (buf[pos] >= 0xf0) {
				pos++;
				continue;
				if (buf[pos] == 0xf0) { /* sysex start */
					sysex = 1;
					pos++;
					continue;
				}
				if (buf[pos] == 0xf7) { /* sysex end */
					sysex = 0;
					pos++;
					continue;
				}
				pos++; /* swallow all other 0xf? commands */
			} else {
				command = buf[pos] >> 4;
				channel = buf[pos] & 0xf;
		
				key = -1;
				velocity = -1;
				h = -1;
				l = -1;
			}
			pos++;
		}

		if (pos == size)
			return 0;
		
//		if (sysex) {
//			pos++;
//			continue;
//		}
		
		switch (command) {
			case 0x8:
			case 0x9:	/* note on */
			case 0xa:	/* aftertouch */
				printf ("blaaa.. %d\n", pos);
				if (key == -1)
					key = buf[pos++];
				if (key == 0) {		/* IGITT! */
					key = -1;
					break;
				}
				if (pos == size) 
					break;
				if (velocity == -1)
					velocity = buf[pos++];

	printf ("key %02x vel %02x\n", key, velocity);
				if (midi_control) {
					if (velocity && key >= KEYSTART_MODE && 
		     					key-KEYSTART_MODE < MAX_MODE) {
						mode = key-KEYSTART_MODE;
						ret = 1;
					}
					else if (velocity && key >= KEYSTART_FLAGS && 
							key-KEYSTART_FLAGS < MAX_FLAGS) {
						flags ^= 1 << (key-KEYSTART_FLAGS);
						ret = 1;
					}
					else if (key >= KEYSTART_DIRECT) {
						int dest = -1;
						key -= KEYSTART_DIRECT;
						if (CHROMATIC_KEYS) {
							if (key <= sizeof (chromatic))
								dest = chromatic[key];
							else dest = -1;
						} else {
							if (key <= KEYSTART_DIRECT + WIDTH)
								dest = key;
							else dest = -1;
						}
						if (dest >= 0) {
							midi_vals[dest][0] = (command == 0x8) ? 0 : velocity * 2;
							midi_vals[dest][1] = (command == 0x8) ? 0 : velocity * 2;
						}
					}
				} else { /* !midi_control */
					printf ("midi_map[%d][%d] ==> %d\n", key, channel, midi_map[key][channel]);
					if (midi_map[key][channel] != -1) {
						midi_vals[midi_map[key][channel]][0] = velocity * 2;
						midi_vals[midi_map[key][channel]][1] = velocity * 2;
					}
				}
				ret = 1;
				key = -1;
				velocity = -1;
				break;
			case 0xb:	/* controllers, sustain */
				if (key == -1)
					key = buf[pos++];
				if (pos == size)
					break;
				if (velocity == -1)
					velocity = buf[pos++];

				printf ("controller %d value %d\n", key, velocity);
				ret = 1;
				key = -1;
				velocity = -1;
				break;
			case 0xc:	/* program change */
				if (key == -1)
					key = buf[pos++];
				if (velocity == -1)
					velocity = buf[pos++];

				key = -1;
				velocity = -1;
				
				break;
			case 0xd:	/* channel key pressure */
				pos++;
				break;
			case 0xe:	/* pitch bend */
				{
					if (l == -1)	
						l = buf[pos++];
					if (pos == size)
						break;
					if (h == -1)
						h = buf[pos++];

					velocity = l | (h << 7);
				
					printf ("vel %d\n", velocity);

					l = -1;
					h = -1;
				}
				break;
		}
	}
	return ret;
}

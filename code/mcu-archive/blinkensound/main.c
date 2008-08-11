/*
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

#include <linux/soundcard.h>
#include <sys/select.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include "values.h"
#include "audio.h"
#include "midi.h"
#include "net.h"
#include "gfx.h"
#include "disp.h"
#include "voodoo.h"
#include "fft.h"

/* globals */
unsigned char audio_vals[WIDTH][2];
unsigned char midi_vals[WIDTH*2][2];
unsigned char matrix[HEIGHT][WIDTH];
int mode = 0, flags = 0;

void usage (const char *name) {
        printf ("Usage: %s [options] hostname ...\n\n", name);
        printf ("Options:\n");
        printf ("  --gfx              activate screen graphics\n");
        printf ("  --dfb-help         show DirectFB usage information\n");
	printf ("  --no-midi-control  disable mode and flag switching via midi equipment\n");
	printf ("  --no-audio         disable reading from /dev/dsp\n");
	printf ("  --midi-map=<chn>,<note>:<slot>  maps midievent <note> on midi channel <chn> to\n");
	printf ("                                  blinkenslot <slot> which can be -1 for 'none'.\n");
	printf ("            (there is a default mapping which is deleted if one --midi-map is given)\n");
        printf ("\n");
}

int main (int argc, char **argv) {
        int update, count=0, digest, n;
        int audio_fd, midi_fd, highest_fd=-1;
        fd_set set;
        struct timeval tv;
	unsigned short int audio_buf[0xffff];
	unsigned char midi_buf[0x2ff];
	int audio_buf_pos = 0, midi_map_given = 0;
	int audio = 1;

        if (argc < 2) {
                usage(argv[0]);
                return EXIT_FAILURE;
        }

	memset (midi_map, -1, sizeof (midi_map));
       
        for (n = 1; n < argc; n++) {
                if (strcmp (argv[n], "--help") == 0) {
                       usage(argv[0]);
                       return EXIT_SUCCESS;
                }
                else if (strcmp (argv[n], "--gfx") == 0)
                        setup_gfx (&argc, &argv);
		else if (strcmp (argv[n], "--no-midi-control") == 0)
			midi_control = 0;
		else if (strcmp (argv[n], "--no-audio") == 0)
			audio = 0;
		else if (strncmp (argv[n], "--midi-map=", strlen ("--midi-map=")) == 0) {
			int chn, note, slot, x, c;
			char *a, *b, *o;
			
			o = strdup (argv[n]);
			
			a = strchr (o, '=');
			a++;
			
			b = strchr (a, ',');
			if (!b) { usage(argv[0]); return EXIT_SUCCESS; }
			*b = '\0';
			chn = strtol (a, NULL, 0);
			b++;

			a = strchr (b, ':');
			if (!a) { usage(argv[0]); return EXIT_SUCCESS; }
			*a = '\0';
			note = strtol (b, NULL, 0);
			a++;
			
			slot = strtol (a, NULL, 0);	
printf ("midi map: %d/%d ==> %d\n", note, chn, slot);	
			if (chn != -1)
				if (note != -1) {
					midi_map[note][chn] = slot;
				} else {
					for (x=0; x<0x80; x++)
						midi_map[x][chn] = slot;
				}
			else
				if (note != -1) {
					for (c=0; c<0x10; c++)
						midi_map[note][c] = slot;
					printf ("bla.9\n");
				} else {
					for (c=0; c<0x10; c++)
						for (x=0; x<0x80; x++)
							midi_map[x][c] = slot;
				}
			
			midi_map_given = 1;
			free (o);
		}
        }

	if (!midi_map_given)
		midi_install_default_map();

        if (net_init ((const char **) argv + 1, argc - 1) < 0) {
                close_gfx();
                return EXIT_FAILURE;
        }

	memset (audio_vals, 0, sizeof (audio_vals));
	memset (midi_vals, 0, sizeof (midi_vals));

        audio_fd = audio_open ();

	if (audio) {
		if (audio_fd < 0) {
			close_gfx();
			return EXIT_FAILURE;
		}
		if (audio_fd > highest_fd)
			highest_fd = audio_fd;
	}
	
	digest = (audio_channels << 12) / audio_buf_size;

	midi_fd = midi_open ();
	if (midi_fd > highest_fd)
		highest_fd = midi_fd;
	
	if (audio_fd > 0)
	        read (audio_fd, audio_buf, 1);
        if (midi_fd > 0)
                read (midi_fd, midi_buf, 0);

	flags = FLAG_FOUNTAIN;
	
	send_packet ();

        while (1) {
                FD_ZERO(&set);
                FD_SET(audio_fd, &set);
                if (midi_fd > 0)
                        FD_SET(midi_fd, &set);
		
                tv.tv_sec  = 0;
                tv.tv_usec = 200000;

		update = 0;

                n = select (highest_fd+1, &set, NULL, NULL, &tv);

                if (n < 0) {
                        if (errno == EINTR)
                                continue;
                        perror ("select");
                        return 1;
                }
		
                if (n == 0 && !(flags & FLAG_PAUSE)) {
                        memset (audio_vals, 0, sizeof(audio_vals));
                        update = 1;
                }

		if (audio && FD_ISSET(audio_fd, &set)) {
			audio_buf_pos += 
				read (audio_fd, audio_buf + audio_buf_pos,
				      (FFT_BUFFER_SIZE*2) - audio_buf_pos);
			if (flags & FLAG_PAUSE)
				audio_buf_pos = 0;
			if (audio_buf_pos == FFT_BUFFER_SIZE*2) {
				audio_parse (audio_buf);
				if (!digest || count++ % digest == 0)
					update = 1;
				audio_buf_pos = 0;
			}
		}

		if (midi_fd > 0 && FD_ISSET(midi_fd, &set)) {
			n = read (midi_fd, midi_buf, sizeof(midi_buf));
			if (n > 0 && midi_parse (midi_buf, n))
				update = 1;
		}

                update |= parse_keys();
		
		if (update) {
                        if (flags & FLAG_CLEAR) {
                                memset (audio_vals, 0, sizeof(audio_vals));
                                flags &= ~FLAG_CLEAR;
                                flags |= FLAG_PAUSE;
                        }
                        //disp_paint ();
                        apply_voodoo ();
                        send_packet ();
                        gfx_show_frame (&matrix[0][0]);

			if (!(flags & FLAG_PAUSE))
				memset (audio_vals, 0, sizeof(audio_vals));

                        //printf ("mode %d flags 0x%02x\n", mode, flags);
                }
        }

        close_gfx();
}


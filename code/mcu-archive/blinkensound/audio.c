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

#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include "audio.h"
#include "fft.h"
#include "values.h"

int audio_open (void) {
        int fd;
        short int buf[0xffff];
        int rate, channels, bits, blocksize, status;

        fd = open (AUDIODEV, O_RDONLY | O_NONBLOCK);
        if (fd < 0) {
                perror ("open "AUDIODEV);
                return -1;
        }

        rate     = 44100;
        channels = 2;
        bits     = 16;

	status = ioctl(fd, SOUND_PCM_WRITE_CHANNELS, &channels);
        if (status ==  -1)
                perror("SOUND_PCM_WRITE_CHANNELS ioctl failed");
        status = ioctl(fd, SOUND_PCM_WRITE_BITS, &bits);
        if (status ==  -1)
                perror("SOUND_PCM_WRITE_BITS ioctl failed");
	status = ioctl(fd, SOUND_PCM_WRITE_RATE, &rate);
        if (status ==  -1)
                perror("SOUND_PCM_WRITE_RATE ioctl failed");
        
        /* get */

        status = ioctl(fd, SOUND_PCM_READ_RATE, &rate);
        if (status ==  -1)
                perror("SOUND_PCM_READ_RATE ioctl failed");
        status = ioctl(fd, SOUND_PCM_READ_CHANNELS, &channels);
        if (status ==  -1)
                perror("SOUND_PCM_READ_CHANNELS ioctl failed");
        status = ioctl(fd, SOUND_PCM_READ_BITS, &bits);
	if (status ==  -1)
		perror("SOUND_PCM_READ_BITS ioctl failed");
	status = ioctl(fd, SNDCTL_DSP_GETBLKSIZE, &blocksize);
	if (status ==  -1)
		perror("SNFCTL_DSP_GETBLKSIZE ioctl failed");


	printf ("Audio device settings:\n"
		"  sampling rate: %d Hz\n"
		"  channels: %d\n"
		"  sample size: %d bits\n"
		"  block size: %d bytes\n",
		rate, channels, bits, blocksize);

        //	blocksize = 256;
	audio_buf_size = blocksize;
	audio_channels = channels;
		
	if (blocksize > sizeof (buf)) {
		printf ("blocksize too big!\n");
		exit (-1);
	}
		
	return fd;
}

int audio_parse (short int *buf) {
	static fft_state *state[2] = { NULL, NULL };
	float out[FFT_BUFFER_SIZE / 2 + 1];
 	int i, y, c, n;

//	int hscale[] = { 0, 1, 3, 7, 10, 15, 20, 27, 34, 44, 56, 70, 88, 110, 136, 168, 256 };
	int hscale[] = { 0, 1, 2, 4, 7, 10, 15, 20, 27, 34, 44, 56, 70, 88, 110, 136, 256 };
	int vscale[] = { 20, 18, 18, 18, 20, 20, 30, 34, 40, 45, 50, 56, 60, 62, 64, 68, 72 };
	
	for (n=0; n<2; n++) {
		if (!state[n])
			state[n] = fft_init();
		
		memset (out, 0, sizeof(out));
		fft_perform (buf+n, out, state[n]);
		for (i=0; i < WIDTH; i++) {
			for (y = 0, c = hscale[i]; c < hscale[i+1]; c++) {
				int sq = ((int)sqrt(out[c]) * vscale[i]) >> 20;

				if (y < sq)
					y = sq;
			}
			
			if (y != 0)
				y = log(y) * FFT_BUFFER_SIZE / log(FFT_BUFFER_SIZE);
		
			if (y > 0xff)
				y = 0xff;
		
			if (y > audio_vals[i][n])
				audio_vals[i][n] = y;
		}
	}
	
	return 1;
}


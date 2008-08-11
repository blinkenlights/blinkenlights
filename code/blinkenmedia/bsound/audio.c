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

#include "config.h"

#include <sys/ioctl.h>

#ifdef HAVE_ESD
#include <esd.h>
#else
#include <linux/soundcard.h>
#endif

#include <stdio.h>
#include <math.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <string.h>

#include <glib.h>
#include <blib/blib.h>

#include "audio.h"
#include "fft.h"
#include "bsound.h"

static gint  rate = 8192;
static gint  channel = 1;
static gint  bits = 16;
static guint blocksize, audio_buf_size, audio_channels;

gint 
b_audio_open (void) 
{
  gint fd = -1;

#ifdef HAVE_ESD
  esd_format_t format = ESD_STREAM | ESD_RECORD | ESD_BITS16 | ESD_STEREO;
  esd_server_info_t *info;
  esd_format_t fmt;
  gint esd;

  if ((esd = esd_open_sound (NULL)) >= 0)
    {
      g_print ("esd sound successfully opened!\n");
      info = esd_get_server_info (esd);
      rate = info->rate;
      fmt = info->format;
      esd_free_server_info (info);
      esd_close (esd);
    }
  else
    {
      rate = esd_audio_rate;
      fmt = esd_audio_format;
    }

  /*
  format = AUDIO_FORMAT_UNSIGNED_8;
  if ((fmt & ESD_MASK_BITS) == ESD_BITS16)
    esd_format |= AUDIO_FORMAT_SIGNED_16;
  esd_channels = fmt & ESD_MASK_CHAN;
  */

  fd = esd_record_stream_fallback (format, rate, NULL, "bsound");
  return (fd);

#else

  /* LINUX/OSS */

  gint status, channels;
  guchar foo[3];	
 
  
  /* we assume linux */

  fd = open (AUDIODEV, O_RDONLY | O_NONBLOCK);
  if (fd < 0) 
    {
      perror ("open "AUDIODEV);
      return -1;
    }
    
  status = ioctl (fd, SOUND_PCM_WRITE_CHANNELS, &channels);
  if (status ==  -1)
    perror ("SOUND_PCM_WRITE_CHANNELS ioctl failed");
  status = ioctl (fd, SOUND_PCM_WRITE_BITS, &bits);
  if (status ==  -1)
    perror ("SOUND_PCM_WRITE_BITS ioctl failed");
  status = ioctl (fd, SOUND_PCM_WRITE_RATE, &rate);
  if (status ==  -1)
    perror("SOUND_PCM_WRITE_RATE ioctl failed");
  
  /* get */
  
  status = ioctl (fd, SOUND_PCM_READ_RATE, &rate);
  if (status ==  -1)
    perror ("SOUND_PCM_READ_RATE ioctl failed");
  status = ioctl (fd, SOUND_PCM_READ_CHANNELS, &channels);
  if (status ==  -1)
    perror ("SOUND_PCM_READ_CHANNELS ioctl failed");
  status = ioctl (fd, SOUND_PCM_READ_BITS, &bits);
  if (status ==  -1)
    perror ("SOUND_PCM_READ_BITS ioctl failed");
  status = ioctl (fd, SNDCTL_DSP_GETBLKSIZE, &blocksize);
  if (status ==  -1)
    perror ("SNFCTL_DSP_GETBLKSIZE ioctl failed");
  
  
  printf ("Audio device settings:\n"
	  "  sampling rate: %d Hz\n"
	  "  channels: %d\n"
	  "  sample size: %d bits\n"
	  "  block size: %d bytes\n",
	  rate, channels, bits, blocksize);
  
  //	blocksize = 256;
  audio_buf_size = blocksize;
  audio_channels = channels;
  
  if (blocksize > 0x7fff) 
    {
      printf ("blocksize too big!\n");
      exit (-1);
    }

  /* workaround for nasty bugs in kernel sound drivers: try to read 1 bytes. */
  read (fd, foo, 1);
  
  return fd;
#endif
}

gint 
b_audio_parse (gshort  *buf,
	       guchar  *audio_vals,
	       guint    width)
{
  static fft_state *state[2] = { NULL, NULL };
  gfloat out[FFT_BUFFER_SIZE / 2 + 1];
  gint i, y, c, n;
  
  gint hscale[] = { 1, 2, 3, 4, 5, 6, 7, 8, 12, 16, 20, 25, 31, 37, 43, 48, 53, 66, 73, 84, 95, 110, 138, 150, 180, 223, 256, 256 };
//  gint hscale[] = { 10, 20, 30, 40, 50, 60, 70, 80, 90, 100, 110, 120, 130, 140, 150, 160, 170, 180, 190, 200, 210, 220, 230, 240, 250, 256, 256 };
//  gint vscale[] = { 28, 22, 22, 22, 22, 22, 30, 30, 30, 40, 50, 60, 60, 60, 64, 80, 100, 100, 100, 110, 110, 164, 164, 164, 164, 164, 164 };
  //gint vscale[] = { 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32 };
  gint vscale[] = { 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 32, 48, 64, 80, 90, 100, 112, 123, 140, 150, 160, 170, 180, 190, 200 };

  memset (audio_vals, 0, WIDTH);
  
  for (n=0; n<1; n++) 
    {
      if (!state[n])
	state[n] = fft_init();
      
      memset (out,        0, sizeof (out));

/*
      audio_vals[0] = 0;
      audio_vals[1] = 0x7f;
      audio_vals[2] = 0xff;

      return 1;
  */    

      memset (audio_vals, 0, WIDTH);

      fft_perform (buf+n, out, state[n]);
      for (i=0; i < WIDTH; i++) 
	{
	  for (y = 0, c = hscale[i]; c < hscale[i+1]; c++) 
	    {
	      gint sq = ((gint) sqrt (out[c]) * vscale[i]) >> 20;
	      
	      if (y < sq)
		y = sq;
	    }
      
	  if (y != 0)
	    y = log (y) * FFT_BUFFER_SIZE / log (FFT_BUFFER_SIZE);
	  
	  if (y > 0xff)
	    y = 0xff;
	  
	  if (y > audio_vals[i])
	    audio_vals[i] = y;
	}
    }
  
  return 1;
}

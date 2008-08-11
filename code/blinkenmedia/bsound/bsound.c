/* bsound - sample audio and/or midi data and do blinken things with it.
 *
 * Copyright (c) 2001-2002  Daniel Mack <daniel@yoobay.net>
 *                          Sven Neumann <sven@gimp.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/time.h>

#ifdef HAVE_GETOPT_LONG
#define _GNU_SOURCE
#include <getopt.h>
#endif

#include <blib/blib.h>

#include "bsound.h"
#include "fft.h"
#include "audio.h"
#include "midi.h"
#include "voodoo.h"

#define SAMPLE_LENGTH 128

#ifdef HAVE_GETOPT_LONG
static const struct option options[] =
{
  { "help",    no_argument,       NULL, 'h' },
  { "version", no_argument,       NULL, 'v' },
  { "audio",   no_argument,       NULL, 'a' },
  { "midi",    no_argument,       NULL, 'm' },
  { NULL,      0,                 NULL,  0  }
};
#endif

static const gchar *option_str = "hvam";

guchar      *audio_vals[AUDIO_RBUF_SIZE];
guchar       midi_vals[WIDTH];
guchar       matrix[HEIGHT][WIDTH];
guchar       audio_peak[WIDTH];
guchar       audio_peak_timeout[WIDTH];
guchar       audio_peak_color[WIDTH];

static void
usage (const gchar *name)
{
  g_printerr ("\n");
  g_printerr ("bsound - sample audio and/or midi data and do blinken things with it.\n"); 
  g_printerr ("Check http://www.blinkenlights.de/ for more information.\n\n");
  g_printerr ("Usage: %s <host1> [<host2:port> ...]\n",
              name);
  g_printerr ("Options:\n");
  g_printerr ("   -h, --help              output usage information\n");
  g_printerr ("   -v, --version           output version information\n");
  g_printerr ("   -a, --audio             sample audio data\n");
  g_printerr ("   -m, --midi              sample midi data\n");
  g_printerr ("\n");
}

int 
main (int   argc,
      char *argv[])
{
  BSender     *sender;
  GError      *error = NULL;
  gint         c, i, n;
  gint         audio_fd = -1, midi_fd = -1;
  fd_set       set;
  gint         highest_fd = -1;
  struct timeval tv;
  gboolean     update;
  gint         audio_buf_pos = 0;
  gshort       audio_buf[0xffff];
  guchar       midi_buf[0xf];
  gint         audio_rbuf_pos = 0;
  gint         digest = 0, count = 0;

  g_type_init ();

  memset (audio_peak, HEIGHT-1, WIDTH);
  memset (audio_peak_timeout, 0, WIDTH);
  
  for (i = 0; i < AUDIO_RBUF_SIZE; i++)
      audio_vals[i] = g_new0 (guchar, WIDTH);
  
#ifdef HAVE_GETOPT_LONG
  while ((c = getopt_long (argc, argv, option_str, options, NULL)) >= 0)
#else
  while ((c = getopt (argc, argv, option_str)) >= 0)
#endif
    {
      switch (c)
        {
        case 'h':
          usage (argv[0]);
          return EXIT_SUCCESS;

        case 'v':
          g_printerr ("bsound (%s version %s)\n", PACKAGE, VERSION);
          return EXIT_SUCCESS;

	case 'a':
	  audio_fd = b_audio_open ();
	  g_print ("sampling audio data (fd=%d)\n", audio_fd);
	  break;

	case 'm':
	  g_print ("sampling midi input.\n");
	  break;

        default:
          usage (argv[0]);
          return EXIT_FAILURE;
        }
    }

  if (argc - optind < 1)
    {
      usage (argv[0]);
      return EXIT_FAILURE;
    }

  sender = b_sender_new ();

  for (i = optind, n = 0; i < argc; i++)
    {
      gchar *colon = strrchr (argv[i], ':');
      gint   port = DEFAULT_PORT;
      
      if (colon)
        {
          b_parse_int (colon + 1, &port);
          *colon = '\0';
        }

      if (b_sender_add_recipient (sender, argv[i], port, &error))
        n++;
      else
        g_printerr ("Skipping %s: %s\n", argv[i], error->message);

      g_clear_error (&error);
    }

  if (!n)
    {
      g_printerr ("all hosts failed, exiting\n");
      g_object_unref (sender);
      return EXIT_FAILURE;
    }

  b_sender_configure (sender,
                      WIDTH, HEIGHT,
                      1, 0xff);

  g_print ("Sending to %d host(s) ...\n", n);

  while (1)
    {
      FD_ZERO(&set);

      highest_fd = -1;
   
      if (audio_fd > 0)
	{
	  FD_SET(audio_fd, &set);
	  if (audio_fd > highest_fd)
	    highest_fd = audio_fd;
	}
      if (midi_fd > 0)
	{
	  FD_SET(midi_fd, &set);
	  if (midi_fd > highest_fd)
	    highest_fd = midi_fd;
	}

      tv.tv_sec  = 1;
      tv.tv_usec = 0;

      update = FALSE;
      
      n = select (highest_fd+1, &set, NULL, NULL, &tv);

      if (n < 0) 
	{
	  if (errno == EINTR)
	    continue;
	  perror ("select");
	  return 1;
	}

      if (n == 0)
	{
	  g_warning ("select() timed out, highest_fd=%d.\n", highest_fd);
//	  memset (audio_vals, 0, sizeof (audio_vals));
	  update = TRUE;
	}

#if 0		
      if (n == 0 && !(flags & FLAG_PAUSE)) 
	{
	  memset (audio_vals, 0, sizeof (audio_vals));
	  update = TRUE;
	}
#endif
 
      if (audio_fd > 0) 
	{
		/*
	  audio_buf_pos += 
	    read (audio_fd, audio_buf + audio_buf_pos,
		  (FFT_BUFFER_SIZE*2) - audio_buf_pos);
	  
	  if (audio_buf_pos == FFT_BUFFER_SIZE * 2) 
*/
		audio_buf_pos += 
	   		read (audio_fd, audio_buf + audio_buf_pos,
					(SAMPLE_LENGTH) - audio_buf_pos);
		
	   if (audio_buf_pos == SAMPLE_LENGTH)
		{
		    /*
	      for (n=0; n < FFT_BUFFER_SIZE; n++)
		g_print ("%04x ", audio_buf[n]);
	      g_print ("\n");
*/
	      b_audio_parse (audio_buf, audio_vals[audio_rbuf_pos], WIDTH);
	      audio_rbuf_pos++;
	      audio_rbuf_pos %= AUDIO_RBUF_SIZE;
	      
	      if (!digest || count++ % digest == 0)
		update = TRUE;
	      audio_buf_pos = 0;
	      memset (audio_buf, 0, sizeof (audio_buf));
	    }
	}
      
      if (midi_fd > 0)
	{
	  n = read (midi_fd, midi_buf, sizeof (midi_buf));
	  //	  if (n > 0 && midi_parse (midi_buf, n))
	  //  update = TRUE;
	}

      apply_voodoo ();

      if (update)
	{
	  b_sender_send_frame (sender, &matrix[0][0]);
	}
    }

  g_object_unref (sender);

  return EXIT_SUCCESS;
}

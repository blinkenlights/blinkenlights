/* bsend - plays Blinkenlights movie files over the net
 *
 * Copyright (c) 2001-2002  Sven Neumann <sven@gimp.org>
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

#ifdef HAVE_GETOPT_LONG
#define _GNU_SOURCE
#include <getopt.h>
#endif

#include <blib/blib.h>

#ifdef G_OS_WIN32
#include <Windows.h>
#endif


#ifdef HAVE_GETOPT_LONG
static const struct option options[] =
{
  { "host",    required_argument, NULL, 'h' },
  { "port",    required_argument, NULL, 'p' },
  { "loop",    no_argument,       NULL, 'l' },
  { "loops",   required_argument, NULL, 'L' },
  { "reverse", no_argument,       NULL, 'r' },
  { "speed",   required_argument, NULL, 's' },
  { "verbose", no_argument,       NULL, 'v' },
  { "help",    no_argument,       NULL, '?' },
  { "version", no_argument,       NULL, 'V' },
  { NULL,      0,                 NULL,  0  }
};
#endif

static const gchar *option_str = "h:p:lL:rs:v?V";


static void
usage (const gchar *name)
{
  g_printerr ("\n");
  g_printerr ("bsend - reads Blinkenlights movies and sends them over the net.\n");
  g_printerr ("Check http://www.blinkenlights.de/ for more information.\n\n");
  g_printerr ("Usage: %s [options] <filename> ...\n",
              name);
  g_printerr ("Options:\n");
  g_printerr ("   -h, --host <hostname>[:port]  destination host\n");
  g_printerr ("   -p, --port                    default destination port\n");
  g_printerr ("   -r, --reverse                 play movie backwards\n");
  g_printerr ("   -l, --loop                    repeat the movie in an endless loop\n");
  g_printerr ("   -L, --loops <number>          repeat the movie as often as specified\n");
  g_printerr ("   -s, --speed <percentage>      adjust movie speed\n");
  g_printerr ("   -v, --verbose                 be verbose\n");
  g_printerr ("   -?, --help                    output usage information\n");
  g_printerr ("   -V, --version                 output version information\n");
  g_printerr ("\n");

#ifndef HAVE_GETOPT_LONG
  g_printerr ("This version of bsend was compiled without support for long command-line\n");
  g_printerr ("options. Only the short, one-letter options will work.\n\n");
#endif
}


int
main (int   argc,
      char *argv[])
{
  BSender     *sender;
  BMovie      *movie;
  const gchar *hostname     = NULL;
  gint         default_port = MCU_LISTENER_PORT;
  GError      *error        = NULL;
  gdouble      speed        = 1.0;
  gint         loops        = 1;
  gboolean     reverse      = FALSE;
  gboolean     verbose      = FALSE;
  gint         n            = 0;
  gint         i, c;

  b_init ();

  sender = b_sender_new ();

#ifdef HAVE_GETOPT_LONG
  while ((c = getopt_long (argc, argv, option_str, options, NULL)) >= 0)
#else
  while ((c = getopt (argc, argv, option_str)) >= 0)
#endif
    {
      switch (c)
        {
        case 'h':
          {
            gchar *colon;
            gint   port = default_port;

            hostname = optarg;

            if ((colon = strchr (optarg, ':')) != NULL)
              {
                *colon = '\0';
                colon++;
                b_parse_int (colon, &port);
              }

            if (b_sender_add_recipient (sender, -1, optarg, port, &error))
              {
                n++;
              }
            else
              {
                g_printerr (error->message);
                return EXIT_FAILURE;
              }
          }

        case 'p':
          b_parse_int (optarg, &default_port);
          break;

        case 'l':
          loops = -1;
          break;

        case 'L':
          b_parse_int (optarg, &loops);
          break;

        case 'r':
          reverse = TRUE;
          break;

        case 's':
          if (!b_parse_double (optarg, &speed) || speed < 0.0)
            {
              g_printerr ("Invalid argument (speed)\n");
              return EXIT_FAILURE;
            }
          break;

        case 'v':
          verbose = TRUE;
          break;

        case '?':
          usage (argv[0]);
          return EXIT_SUCCESS;

        case 'V':
          g_printerr ("bsend (%s version %s)\n", PACKAGE, VERSION);
          return EXIT_SUCCESS;

        default:
          usage (argv[0]);
          return EXIT_FAILURE;
        }
    }

  b_sender_set_verbose (sender, verbose);

  if (! hostname)
    {
      if (verbose)
        g_print ("No host specified, using localhost.\n");

      if (b_sender_add_recipient (sender, -1,
                                  "localhost", MCU_LISTENER_PORT, NULL))
        n++;
    }

  if (! n)
    {
      g_printerr ("All hosts failed, exiting\n");
      g_object_unref (sender);
      return EXIT_FAILURE;
    }

  for (n = 0; n == 0 || optind < argc; n++, optind++)
    {
      const gchar *filename = optind < argc ? argv[optind] : NULL;

      if ((n== 0 && ! filename) || strcmp (filename, "-") == 0)
        {
          movie = b_movie_new_from_fd (0, &error);
          filename = "<stdin>";
        }
      else
        {
          movie = b_movie_new_from_file (filename, FALSE, &error);
        }

      if (! movie)
        {
          g_printerr ("Error opening '%s': %s\n", filename, error->message);
          g_clear_error (&error);
          continue;
        }

      if (reverse || speed != 1.0)
        {
          BEffects *effects = b_effects_new ();

          b_movie_apply_effects (movie, effects, reverse, speed);

          g_object_unref (effects);
        }

      if (verbose)
        g_print ("Loaded '%s' (%dx%d) channels=%d maxval=%d (%d frames, %d.%d s)\n",
                 movie->title ? movie->title : b_object_get_name (B_OBJECT (movie)),
                 movie->width, movie->height, movie->channels, movie->maxval,
                 movie->n_frames,
                 movie->duration / 1000, (movie->duration % 1000) / 10);

      b_sender_configure (sender,
                          movie->width, movie->height,
                          movie->channels, movie->maxval);

      for (i = 0; loops < 0 || i < loops; i++)
        {
          GList *list;

          for (list = movie->frames; list; list = list->next)
            {
              BMovieFrame *frame = list->data;

              b_sender_send_frame (sender, frame->data);

#ifdef G_OS_WIN32
              Sleep (frame->duration);
#else
              usleep (frame->duration * 1000);
#endif
            }
        }

      g_object_unref (movie);
    }

  g_object_unref (sender);

  return EXIT_SUCCESS;
}

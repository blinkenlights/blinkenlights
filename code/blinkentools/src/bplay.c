/* bplay - plays Blinkenlights movie files on the console
 *
 * Copyright (c) 2001-2003  Sven Neumann <sven@gimp.org>
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

#include <aalib.h>

#ifdef HAVE_GETOPT_LONG
#define _GNU_SOURCE
#include <getopt.h>
#endif

#include <blib/blib.h>
#include <blib/bview-aa.h>

#ifndef G_OS_WIN32
#include <signal.h>
#endif

#ifdef HAVE_GETOPT_LONG
static const struct option options[] =
{
  { "loop",    no_argument, NULL, 'l' },
  { "help",    no_argument, NULL, '?' },
  { "version", no_argument, NULL, 'V' },
  { NULL,      0,           NULL,  0  }
};
#endif

static const gchar *option_str = "l?V";

static GMainLoop   *main_loop  = NULL;


static void
usage (const gchar *name)
{
  g_printerr ("bplay - plays Blinkenlights movies on the text console.\n");
  g_printerr ("Check http://www.blinkenlights.de/ for more information.\n\n");
  g_printerr ("Usage: %s [options] <filename>\n",
              name);
  g_printerr ("Options:\n");
  g_printerr ("   -l, --loop       repeat the movie in an endless loop\n");
  g_printerr ("   -?, --help       output usage information\n");
  g_printerr ("   -V, --version    output version information\n");
  g_printerr ("\n");

#ifndef HAVE_GETOPT_LONG
  g_printerr ("This version of bplay was compiled without support for long command-line\n");
  g_printerr ("options. Only the short, one-letter options will work.\n\n");
#endif
}

static gboolean
paint (BModule  *module,
       guchar   *buffer,
       gpointer  data)
{
  b_view_aa_update (B_VIEW_AA (data), buffer);

  return TRUE;
}

#ifndef G_OS_WIN32
static void
sigint_handler (gint signum)
{
  sigset_t sigset;

  sigemptyset (&sigset);
  sigprocmask (SIG_SETMASK, &sigset, NULL);

  if (main_loop)
    g_main_loop_quit (main_loop);
}
#endif

int
main (int   argc,
      char *argv[])
{
  BModule     *module;
  BMovie      *movie;
  BViewAA     *view;
  aa_context  *context;
  const gchar *filename;
  GError      *error = NULL;
  gboolean     loop  = FALSE;
  gint         c;

  b_init ();

#ifdef HAVE_GETOPT_LONG
  while ((c = getopt_long (argc, argv, option_str, options, NULL)) >= 0)
#else
  while ((c = getopt (argc, argv, option_str)) >= 0)
#endif
    {
      switch (c)
        {
        case 'l':
          loop = TRUE;
          break;

        case '?':
          usage (argv[0]);
          return EXIT_SUCCESS;

        case 'V':
          g_printerr ("bplay (%s version %s)\n", PACKAGE, VERSION);
          return EXIT_SUCCESS;

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

  filename = argv[optind];

  if (strcmp (filename, "-") == 0)
    {
      filename = "<stdin>";
      movie = b_movie_new_from_fd (0, &error);
    }
  else
    {
      movie = b_movie_new_from_file (filename, FALSE, &error);
    }

  if (!movie)
    {
      g_printerr ("Error opening '%s': %s\n", filename, error->message);
      return EXIT_FAILURE;
    }

  context = aa_autoinit (&aa_defparams);
  if (!context)
    {
      g_printerr ("Error creating an aalib context\n");
      return EXIT_FAILURE;
    }

  aa_hidecursor (context);
  aa_autoinitkbd (context, 0);
  aa_resizehandler (context, (void *) aa_resize);

  view = b_view_aa_new (context,
                        movie->height, movie->width, movie->channels, &error);
  if (!context)
    {
      g_printerr ("%s\n", error->message);
      return EXIT_FAILURE;
    }

  module = b_module_new (B_TYPE_MOVIE_PLAYER,
                         movie->width, movie->height, NULL,
                         paint, view,
                         &error);

  if (!module)
    {
      g_printerr ("%s\n", error->message);
      return EXIT_FAILURE;
    }

  g_object_set (module, "movie", filename, NULL);

  /* hack! there should be an API to do this */
  B_MOVIE_PLAYER (module)->movie = movie;
  module->ready = TRUE;

  main_loop = g_main_loop_new (NULL, FALSE);

  if (loop)
    {
      g_signal_connect (G_OBJECT (module), "stop",
                        G_CALLBACK (b_module_start),
                        NULL);
    }
  else
    {
      g_signal_connect_swapped (G_OBJECT (module), "stop",
                                G_CALLBACK (g_main_loop_quit),
                                main_loop);
    }

#ifndef G_OS_WIN32
  {
    struct sigaction  sa;

    /* handle SIGINT */
    sigfillset (&sa.sa_mask);
    sa.sa_handler = sigint_handler;
    sa.sa_flags   = SA_RESTART;
    sigaction (SIGINT, &sa, NULL);
  }
#endif

  b_module_start (module);

  g_main_loop_run (main_loop);

  g_main_loop_unref (main_loop);
  g_object_unref (module);
  aa_close (context);
  g_object_unref (view);

  return EXIT_SUCCESS;
}

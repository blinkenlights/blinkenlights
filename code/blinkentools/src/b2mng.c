/* b2mng
 * Creates MNG animations from Blinkenlights movies.
 *
 * Copyright (C) 2002-2004  Sven Neumann <sven@gimp.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>

#include <unistd.h>
#include <string.h>

#ifdef HAVE_GETOPT_LONG
#define _GNU_SOURCE
#include <getopt.h>
#endif

#include <blib/blib.h>

#include "bmovie-mng.h"


#ifdef HAVE_GETOPT_LONG
static const struct option options[] =
{
  { "theme",   required_argument, NULL, 't' },
  { "output",  required_argument, NULL, 'o' },
  { "loop",    no_argument,       NULL, 'l' },
  { "loops",   required_argument, NULL, 'L' },
  { "verbose", no_argument,       NULL, 'v' },
  { "help",    no_argument,       NULL, '?' },
  { "version", no_argument,       NULL, 'V' },
  { NULL,      0,                 NULL,  0  }
};
#endif

static const gchar *option_str = "t:o:lL:v?V";

static void
usage (const gchar *name)
{
  g_printerr ("b2mng - creates MNG animations from Blinkenlights movies.\n");
  g_printerr ("Check http://www.blinkenlights.de/ for more information.\n\n");
  g_printerr ("Usage: %s [options] <filename>\n", name);
  g_printerr ("Options:\n");
  g_printerr ("   -t, --theme <filename>     specifies theme\n");
  g_printerr ("   -o, --output <filename>    write to file instead of stdout\n");
  g_printerr ("   -l, --loop                 create endlessly looping animation\n");
  g_printerr ("   -L, --loops <number>       loop animation as often as specified\n");
  g_printerr ("   -v, --verbose              be verbose\n");
  g_printerr ("   -?, --help                 output usage information\n");
  g_printerr ("   -v, --version              output version information\n");
  g_printerr ("\n");

#ifndef HAVE_GETOPT_LONG
  g_printerr ("This version of b2mng was compiled without support for long command-line\n");
  g_printerr ("options. Only the short, one-letter options will work.\n\n");
#endif
}


static BTheme *
load_theme (const gchar *name)
{
  BTheme *theme;
  GError *error = NULL;

  g_return_val_if_fail (name != NULL, NULL);

  if (g_file_test (name, G_FILE_TEST_IS_REGULAR))
    {
      theme = b_theme_new_from_file (name, TRUE, &error);
      if (!theme)
        {
          g_printerr ("Error opening '%s': %s\n", name, error->message);
          return NULL;
        }
    }
  else
    {
      theme = b_themes_lookup_theme (name, NULL, &error);
      if (!theme)
        {
          g_printerr ("\n%s\n", error->message);
          g_printerr ("Fix your spelling or try setting the "
                      "B_THEME_PATH environment variable.\n");
          return NULL;
        }
      if (!b_theme_load (theme, &error))
        {
          g_printerr ("Error loading theme '%s': %s\n",
                      b_object_get_name (B_OBJECT (theme)), error->message);
          return NULL;
        }
    }

  return theme;
}

int
main (int   argc,
      char *argv[])
{
  const gchar *movie_name = NULL;
  const gchar *theme_name = NULL;
  const gchar *output     = NULL;
  FILE        *stream;
  gboolean     success;
  gboolean     verbose    = FALSE;
  gint         loops      = 1;
  gint         c;
  BMovie      *movie;
  BTheme      *theme      = NULL;
  GError      *error      = NULL;

  b_init ();

#ifdef HAVE_GETOPT_LONG
  while ((c = getopt_long (argc, argv, option_str, options, NULL)) >= 0)
#else
  while ((c = getopt (argc, argv, option_str)) >= 0)
#endif
    {
      switch (c)
        {
        case 't':
          theme_name = optarg;
          break;

        case 'o':
          output = optarg;
          break;

        case 'l':
          loops = -1;
          break;

        case 'L':
          b_parse_int (optarg, &loops);
          break;

        case 'v':
          verbose = TRUE;
          break;

        case '?':
          usage (argv[0]);
          return EXIT_SUCCESS;

        case 'V':
          g_printerr ("b2mng (%s version %s)\n", PACKAGE, VERSION);
          return EXIT_SUCCESS;

        default:
          usage (argv[0]);
          return EXIT_FAILURE;
        }
    }

  if (optind < argc)
    movie_name = argv[optind];

  if (theme_name)
    {
      theme = load_theme (theme_name);

      if (!theme)
        return EXIT_FAILURE;

      if (verbose)
        g_printerr ("Loaded theme '%s' (%dx%d, %d levels) at size %dx%d\n",
                    b_object_get_name (B_OBJECT (theme)),
                    theme->columns, theme->rows, theme->maxval + 1,
                    theme->width, theme->height);
    }

  if (output && *output)
    stream = fopen (output, "w");
  else
    stream = stdout;

  if (! stream)
    {
      g_printerr ("Can't write to '%s': %s", output, g_strerror (errno));
      return EXIT_FAILURE;
    }

  if (! movie_name || (strcmp (movie_name, "-") == 0))
    {
      movie = b_movie_new_from_fd (0, &error);
      movie_name = "<stdin>";
    }
  else
    {
      movie = b_movie_new_from_file (movie_name, FALSE, &error);
    }

  if (! movie)
    {
      g_printerr ("Error opening '%s': %s\n", movie_name, error->message);
      return EXIT_FAILURE;
    }

  if (verbose)
    g_printerr ("Loaded movie '%s' (%dx%d) maxval=%d (%d frames, %d.%d s)\n",
                movie->title ?
                movie->title : b_object_get_name (B_OBJECT (movie)),
                movie->width, movie->height, movie->maxval,
                movie->n_frames,
                movie->duration / 1000, (movie->duration % 1000) / 10);

  b_movie_normalize (movie, 255);

  success = b_movie_save_as_mng (movie, theme, stream, loops, &error);

  if (output && *output)
    fclose (stream);

  if (success)
    {
      if (verbose)
        g_printerr ("Successfully created MNG\n");
    }
  else
    {
      g_printerr ("Error saving movie '%s': %s\n",
                  (movie->title ?
                   movie->title : b_object_get_name (B_OBJECT (movie))),
                  error->message);
    }

  g_object_unref (movie);
  if (theme)
    g_object_unref (theme);

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

/* b2b.c
 * Converts between various formats for Blinkenlights movies.
 *
 * Copyright (C) 2001-2004  Sven Neumann <sven@gimp.org>
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
#include <string.h>
#include <errno.h>
#include <math.h>

#include <unistd.h>
#include <string.h>

#ifdef HAVE_GETOPT_LONG
#define _GNU_SOURCE
#include <getopt.h>
#endif

#include <blib/blib.h>


#ifdef HAVE_GETOPT_LONG
static const struct option options[] =
{
  { "type",         required_argument, NULL, 't' },
  { "output",       required_argument, NULL, 'o' },
  { "bits",         required_argument, NULL, 'b' },
  { "invert",       no_argument,       NULL, 'i' },
  { "gamma",        required_argument, NULL, 'g' },
  { "hflip",        no_argument,       NULL, 'f' },
  { "vflip",        no_argument,       NULL, 'F' },
  { "reverse",      no_argument,       NULL, 'r' },
  { "speed",        required_argument, NULL, 's' },
  { "fps",          required_argument, NULL, 'c' },
  { "loop-hint",    no_argument,       NULL, 'x' },
  { "no-loop-hint", no_argument,       NULL, 'X' },
  { "meta",         no_argument,       NULL, 'm' },
  { "help",         no_argument,       NULL, '?' },
  { "version",      no_argument,       NULL, 'V' },
  { NULL,           0,                 NULL,  0  }
};
#endif

static const gchar *option_str = "t:o:b:g:ifFrs:c:xXm?V";


static void   b_movie_apply_gamma         (BMovie  *movie,
                                           gdouble  gamma);
static void   b_movie_set_fixed_framerate (BMovie  *movie,
                                           double  fps);

static void
usage (const gchar *name)
{
  g_printerr ("b2b - a format converter for Blinkenlights movies.\n");
  g_printerr ("Check http://www.blinkenlights.de/ for more information.\n\n");
  g_printerr ("Usage: %s [options] <filename>\n", name);
  g_printerr ("Options:\n");
  g_printerr ("   -o, --output <filename>    write to file instead of stdout\n");
  g_printerr ("   -t, --type [blm|bml|gif]   specifies output type\n");
  g_printerr ("   -b, --bits <number>        number of significant bits in output\n");
  g_printerr ("   -i, --invert               invert movie\n");
  g_printerr ("   -g, --gamma <factor>       gamma-adjust all frames\n");
  g_printerr ("   -f, --hflip                flip movie horizontally\n");
  g_printerr ("   -F, --vflip                flip movie vertically\n");
  g_printerr ("   -x, --loop-hint            set loop flag\n");
  g_printerr ("   -X, --no-loop-hint         unset loop flag\n");
  g_printerr ("   -r, --reverse              reverse movie\n");
  g_printerr ("   -s, --speed <factor>       adjust movie speed\n");
  g_printerr ("   -c, --fps <number>         generate <number> frames per second\n");
  g_printerr ("   -m, --meta                 show info about the movie, don't convert it\n");
  g_printerr ("   -?, --help                 output usage information\n");
  g_printerr ("   -V, --version              output version information\n");
  g_printerr ("\n");

#ifndef HAVE_GETOPT_LONG
  g_printerr ("This version of b2b was compiled without support for long command-line\n");
  g_printerr ("options. Only the short, one-letter options will work.\n\n");
#endif
}

static GType
b2b_type_from_name (const gchar *name)
{
  if (g_ascii_strcasecmp (name, "blm") == 0)
    return B_TYPE_MOVIE_BLM;
  if (g_ascii_strcasecmp (name, "bml") == 0)
    return B_TYPE_MOVIE_BML;
  if (g_ascii_strcasecmp (name, "gif") == 0)
    return B_TYPE_MOVIE_GIF;

  return G_TYPE_NONE;
}

int
main (int   argc,
      char *argv[])
{
  const gchar *filename = NULL;
  const gchar *output   = NULL;
  FILE        *stream   = NULL;
  BEffects    *effects  = NULL;
  gboolean     reverse  = FALSE;
  gboolean     meta     = FALSE;
  gdouble      gamma    = 1.0;
  gdouble      speed    = 1.0;
  gdouble      fps      = 0.0;
  gboolean     success;
  gint         loop     = -1;
  gint         bits     = -1;
  gint         c;
  BMovie      *movie;
  GType        type     = G_TYPE_NONE;
  GError      *error    = NULL;

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
          type = b2b_type_from_name (optarg);
          if (type == G_TYPE_NONE)
            {
              usage (argv[0]);
              return EXIT_FAILURE;
            }
          break;

        case 'o':
          output = optarg;
          break;

        case 'b':
          if (! b_parse_int (optarg, &bits) || bits < 1 || bits > 8)
            {
              g_printerr ("Invalid argument (bits)\n");
              return EXIT_FAILURE;
            }
          break;

        case 'i':
          if (! effects)
            effects = b_effects_new ();
          effects->invert = B_EFFECT_SCOPE_ALL;
          break;

        case 'g':
          if (! b_parse_double (optarg, &gamma) || gamma < 0.0)
            {
              g_printerr ("Invalid argument (gamma)\n");
              return EXIT_FAILURE;
            }
          break;

        case 'f':
          if (! effects)
            effects = b_effects_new ();
          effects->hflip = B_EFFECT_SCOPE_ALL;
          break;

        case 'F':
          if (! effects)
            effects = b_effects_new ();
          effects->vflip = B_EFFECT_SCOPE_ALL;
          break;

        case 'r':
          reverse = TRUE;
          break;

        case 's':
          if (! b_parse_double (optarg, &speed) || speed < 0.0)
            {
              g_printerr ("Invalid argument (speed)\n");
              return EXIT_FAILURE;
            }
          break;

        case 'c':
          if (! b_parse_double (optarg, &fps) || fps < 0.0)
            {
              g_printerr ("Invalid argument (fps)\n");
              return EXIT_FAILURE;
            }
          break;

        case 'l':
          loop = FALSE;
          break;

        case 'L':
          loop = TRUE;
          break;

        case 'm':
          meta = TRUE;
          break;

        case '?':
          usage (argv[0]);
          return EXIT_SUCCESS;

        case 'V':
          g_printerr ("b2b (%s version %s)\n", PACKAGE, VERSION);
          return EXIT_SUCCESS;

        default:
          usage (argv[0]);
          return EXIT_FAILURE;
        }
    }

  if (optind < argc)
    filename = argv[optind];

  if (type == G_TYPE_NONE)
    {
      gchar *basename = g_path_get_basename (argv[0]);

      if (strlen (basename) == 5)
        type = b2b_type_from_name (basename + 2);

      g_free (basename);
    }

  if (! meta)
    {
      if (output && *output)
        {
          if (type == G_TYPE_NONE)
            {
              gint len = strlen (output);

              if (len > 4 && output[len-4] == '.')
                type = b2b_type_from_name (output + len - 3);
            }

          stream = fopen (output, "w");
        }
      else
        {
          stream = stdout;
        }

      if (! stream)
        {
          g_printerr ("Can't write to '%s': %s", output, g_strerror (errno));
          return EXIT_FAILURE;
        }
    }

  if (! filename || (strcmp (filename, "-") == 0))
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
      g_return_val_if_fail (error != NULL, EXIT_FAILURE);

      g_printerr ("Error opening '%s': %s\n", filename, error->message);
      return EXIT_FAILURE;
    }

  if (bits > 0)
    {
      b_movie_normalize (movie, (1 << bits) - 1);
    }
  /* when converting from GIF to BML, default to bits = 4 */
  else if (B_IS_MOVIE_GIF (movie) && type == B_TYPE_MOVIE_BML)
    {
      b_movie_normalize (movie, (1 << 4) - 1);
    }

  if (gamma != 1.0)
    b_movie_apply_gamma (movie, gamma);

  if (effects || reverse || speed != 1.0)
    b_movie_apply_effects (movie, effects, reverse, speed);

  if (loop != -1)
    movie->loop = (loop != FALSE);

  if (meta)
    {
      const gchar *type_name =  G_OBJECT_TYPE_NAME (G_OBJECT (movie));

      if (strncmp (type_name, "BMovie", 6) == 0)
        type_name += 6;

      g_print ("Blinkenlights Movie (%s) %dx%d "
               "channels=%d maxval=%d frames=%d duration=%dms\n", type_name,
               movie->width, movie->height,
               movie->channels, movie->maxval,
               movie->n_frames, movie->duration);

      success = TRUE;
    }
  else
    {
      if (fps > 0.0)
        b_movie_set_fixed_framerate (movie, fps);

      if (type != G_TYPE_NONE)
        success = b_movie_save_as (movie, type, stream, &error);
      else
        success = b_movie_save (movie, stream, &error);

      if (output && *output)
        fclose (stream);

      if (! success)
        {
          g_return_val_if_fail (error != NULL, EXIT_FAILURE);

          g_printerr ("Error saving movie '%s': %s\n",
                      (movie->title ?
                       movie->title : b_object_get_name (B_OBJECT (movie))),
                      error->message);
        }
    }

  g_object_unref (movie);

  if (effects)
    g_object_unref (effects);

  return success ? EXIT_SUCCESS : EXIT_FAILURE;
}

static void
b_movie_apply_gamma (BMovie  *movie,
                     gdouble  gamma)
{
  gdouble  ind;
  guchar  *map;
  gint     i;

  if (gamma == 0.0 || movie->maxval < 1)
    return;

  map = g_new (guchar, movie->maxval + 1);

  gamma = 1.0 / gamma;

  for (i = 0; i <= movie->maxval; i++)
    {
      ind = (gdouble) i / (gdouble) movie->maxval;
      map[i] =
        (guchar) (gint) ((gdouble) movie->maxval * pow (ind, gamma) + 0.5);
    }

  b_movie_apply_colormap (movie, map);

  g_free (map);
}

static void
b_movie_set_fixed_framerate (BMovie  *movie,
                             gdouble  fps)
{
  GList *list     = NULL;
  GList *frames   = NULL;
  gint   duration = (1000.0 / fps);
  gint   t;

  for (t = 0; t < movie->duration; t += duration)
    {
      list = b_movie_get_frame_at_time (movie, list, t);

      if (list)
        {
          BMovieFrame *frame = list->data;

          frames = g_list_prepend (frames,
                                   g_memdup (frame->data,
                                             movie->width * movie->height));
        }
    }

  for (list = movie->frames; list; list = g_list_next (list))
    {
      BMovieFrame *frame = list->data;

      g_free (frame->data);
      g_free (frame);
    }

  g_list_free (movie->frames);

  movie->duration = 0;
  movie->n_frames = 0;
  movie->frames   = NULL;

  for (list = frames; list; list = list->next)
    {
      b_movie_prepend_frame (movie, duration, list->data);
      g_free (list->data);
    }

  g_list_free (frames);
}

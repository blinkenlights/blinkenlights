/* b2image
 * Renders an image from a Blinkenlights frame.
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

#include <unistd.h>
#include <errno.h>
#include <string.h>

#ifdef HAVE_GETOPT_LONG
#define _GNU_SOURCE
#include <getopt.h>
#endif

#include <blib/blib.h>
#include <blib/blib-pixbuf.h>


#ifdef HAVE_GETOPT_LONG
static const struct option options[] =
{
  { "theme",   required_argument, NULL, 't' },
  { "output",  required_argument, NULL, 'o' },
  { "format",  required_argument, NULL, 'f' },
  { "index",   required_argument, NULL, 'i' },
  { "all",     no_argument,       NULL, 'a' },
  { "link",    no_argument,       NULL, 'l' },
  { "quality", required_argument, NULL, 'q' },
  { "verbose", no_argument,       NULL, 'v' },
  { "help",    no_argument,       NULL, '?' },
  { "version", no_argument,       NULL, 'V' },
  { NULL,      0,                 NULL,  0  }
};
#endif

static const gchar *option_str = "t:o:f:i:alq:v?V";

static void
usage (const gchar *name)
{
  g_printerr ("b2image - renders an image from a Blinkenlights frame.\n");
  g_printerr ("Check http://www.blinkenlights.de/ for more information.\n\n");
  g_printerr ("Usage: %s [options] <filename>\n", name);
  g_printerr ("Options:\n");
  g_printerr ("   -t, --theme <filename>     specifies theme\n");
  g_printerr ("   -o, --output <filename>    write to file <filename>\n");
  g_printerr ("   -f, --format [png|jpeg]    specifies output file format\n");
  g_printerr ("   -i, --index <number>       dump frame <number> (default: 0)\n");
  g_printerr ("   -a, --all                  dump all frame\n");
  g_printerr ("   -l, --link                 link duplicate frames\n");
  g_printerr ("   -q, --quality              set JPEG quality (0-100)\n");
  g_printerr ("   -v, --verbose              be verbose\n");
  g_printerr ("   -?, --help                 output usage information\n");
  g_printerr ("   -V, --version              output version information\n");
  g_printerr ("\n");

#ifndef HAVE_GETOPT_LONG
  g_printerr ("This version of b2image was compiled without support for long command-line\n");
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

/*  copied from gdk-pixbuf-io.c  */
static void
collect_save_options (va_list   opts,
                      gchar  ***keys,
                      gchar  ***vals)
{
  gchar *key;
  gchar *val;
  gchar *next;
  gint   count = 0;

  *keys = NULL;
  *vals = NULL;

  next = va_arg (opts, gchar *);
  while (next)
    {
      key = next;
      val = va_arg (opts, gchar *);

      ++count;

      *keys = g_renew (gchar *, *keys, count + 1);
      *vals = g_renew (gchar *, *vals, count + 1);

      (*keys)[count - 1] = g_strdup (key);
      (*vals)[count - 1] = g_strdup (val);

      (*keys)[count] = NULL;
      (*vals)[count] = NULL;

      next = va_arg (opts, gchar *);
    }
}

static gboolean
b_view_pixbuf_save_frame_v (BViewPixbuf  *view,
                            BMovieFrame  *frame,
                            const gchar  *filename,
                            gboolean      verbose,
                            const gchar  *type,
                            gchar       **option_keys,
                            gchar       **option_values,
                            GError      **error)
{
  const GdkPixbuf *pixbuf;

  if (! frame)
    return TRUE;

  pixbuf = b_view_pixbuf_render (view, frame->data, NULL, error);

  if (pixbuf &&
      gdk_pixbuf_savev ((GdkPixbuf *) pixbuf,
                        filename, type,
                        option_keys, option_values,
                        error))
    {
      if (verbose)
        g_printerr ("Wrote %s\n", filename);

      return TRUE;
    }

  return FALSE;
}

static gboolean
b_view_pixbuf_save_frame (BViewPixbuf  *view,
                          BMovieFrame  *frame,
                          const gchar  *filename,
                          const gchar  *type,
                          gboolean      verbose,
                          GError      **error,
                          ...)
{
  gchar    **keys   = NULL;
  gchar    **values = NULL;
  gboolean   success;
  va_list    args;

  va_start (args, error);
  collect_save_options (args, &keys, &values);
  va_end (args);

  success = b_view_pixbuf_save_frame_v (view,
                                        frame, filename, verbose, type,
                                        keys, values,
                                        error);

  g_strfreev (keys);
  g_strfreev (values);

  return success;
}

static gboolean
b_view_pixbuf_save_movie (BViewPixbuf  *view,
                          BMovie       *movie,
                          const gchar  *basename,
                          const gchar  *type,
                          gboolean      hardlink,
                          gboolean      verbose,
                          GError      **error,
                          ...)
{
  static const guchar *last_data = NULL;

  gsize     len      = strlen (basename);
  gchar    *filename = g_alloca (len + 16 + strlen (type));
  gchar    *oldname  = g_alloca (len + 16 + strlen (type));
  gchar    *suffix   = filename + len;
  gchar    *oldfix   = oldname + len;
  gchar   **keys     = NULL;
  gchar   **values   = NULL;
  GList    *list;
  gint      n;
  va_list   args;

  strcpy (filename, basename);
  strcpy (oldname, basename);

  va_start (args, error);
  collect_save_options (args, &keys, &values);
  va_end (args);

  for (list = movie->frames, n = 1; list; list = list->next, n++)
    {
      BMovieFrame *frame = list->data;

      sprintf (suffix, ".%05d.%s", n, type);

      if (hardlink  &&
          last_data &&
          memcmp (frame->data, last_data, movie->width * movie->height) == 0)
        {
          sprintf (oldfix, ".%05d.%s", n - 1, type);

          if (link (oldname, filename) != 0)
            {
              g_set_error (error, 0, 0,
                           "Could not create link for '%s': %s\n"
                           "Perhaps try again without the --link option.",
                           filename, g_strerror (errno));
              return FALSE;
            }
        }
      else
        {
          if (! b_view_pixbuf_save_frame_v (view,
                                            list->data,
                                            filename, verbose, type,
                                            keys, values,
                                            error))
            return FALSE;
        }

      last_data = frame->data;
    }

  g_strfreev (keys);
  g_strfreev (values);

  return TRUE;
}

int
main (int   argc,
      char *argv[])
{
  const gchar *movie_name = NULL;
  const gchar *theme_name = NULL;
  const gchar *output     = NULL;
  const gchar *quality    = NULL;
  const gchar *format     = NULL;
  gboolean     success;
  gboolean     hardlink   = FALSE;
  gboolean     verbose    = FALSE;
  gboolean     all        = FALSE;
  gint         index      = 1;
  gint         c;
  BViewPixbuf *view       = NULL;
  BMovie      *movie;
  BTheme      *theme      = NULL;
  GError      *error      = NULL;
  gchar       *basename   = g_path_get_basename (argv[0]);

  if (strcmp (basename, "b2image") &&
      strlen (basename) > 4        &&
      basename[0] == 'b' && basename[1] == '2')
    {
      format = basename + 2;
    }

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

        case 'f':
          format = optarg;
          break;

        case 'i':
          b_parse_int (optarg, &index);
          break;

        case 'a':
          all = TRUE;
          break;

        case 'q':
          quality = optarg;
          break;

        case 'l':
          hardlink = TRUE;
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

  if (! output)
    {
      g_printerr ("Need to specify an output file using the --output command-line option.\n");
      return EXIT_FAILURE;
    }

  if (! format)
    {
      g_printerr ("Need to specify an output file format using the --format command-line option.\n");
      return EXIT_FAILURE;
    }

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

  if (theme)
    {
      if (movie->width != theme->columns || movie->height != theme->rows)
        {
          g_printerr ("Movie and theme dimensions do not match!\n");
          return EXIT_FAILURE;
        }

      view = b_view_pixbuf_new_theme (theme, FALSE, &error);
    }
  else
    {
      view = b_view_pixbuf_new (movie->height, movie->width, movie->channels,
                                &error);
    }

  if (! view)
    {
      g_printerr ("%s\n", error->message);
      return EXIT_FAILURE;
    }

  if (all)
    success = b_view_pixbuf_save_movie (view,
                                        movie,
                                        output, format, hardlink, verbose,
                                        &error,
                                        quality ? "quality" : NULL, quality,
                                        NULL);
  else
    success = b_view_pixbuf_save_frame (view,
                                        g_list_nth_data (movie->frames,
                                                         index - 1),
                                        output, format, verbose,
                                        &error,
                                        quality ? "quality" : NULL, quality,
                                        "quality", quality,
                                        NULL);

  if (! success)
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

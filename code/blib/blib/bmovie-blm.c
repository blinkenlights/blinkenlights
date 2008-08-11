/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
 *                          Michael Natterer <mitch@gimp.org>
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

#include <string.h>

#include <glib-object.h>

#include "btypes.h"
#include "bmovie-blm.h"


static void       b_movie_blm_class_init        (BMovieBLMClass  *class);
static void       b_movie_blm_init              (BMovie          *movie);

static gboolean   b_movie_blm_load_info         (BMovie          *movie,
                                                 GIOChannel      *io,
                                                 GError         **error);
static gboolean   b_movie_blm_load_all          (BMovie          *movie,
                                                 GIOChannel      *io,
                                                 GError         **error);
static gboolean   b_movie_blm_save              (BMovie          *movie,
                                                 FILE            *stream,
                                                 GError         **error);

static gboolean   b_movie_blm_parse_header_line (BMovie          *movie,
                                                 const gchar     *buf,
                                                 gsize            len,
                                                 gboolean        *magic);

static void       b_movie_blm_save_header       (FILE            *stream,
                                                 const gchar     *name, 
                                                 const gchar     *value);

static BMovieClass *parent_class = NULL;


GType
b_movie_blm_get_type (void)
{
  static GType movie_type = 0;

  if (!movie_type)
    {
      static const GTypeInfo movie_info =
      {
        sizeof (BMovieBLMClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_movie_blm_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BMovieBLM),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_movie_blm_init
      };

      movie_type = g_type_register_static (B_TYPE_MOVIE,
                                           "BMovieBLM", 
                                           &movie_info, 0);
    }

  return movie_type;
}

static void
b_movie_blm_class_init (BMovieBLMClass *class)
{
  BMovieClass *movie_class;

  parent_class = g_type_class_peek_parent (class);

  movie_class  = B_MOVIE_CLASS (class);

  movie_class->load_info = b_movie_blm_load_info;
  movie_class->load_all  = b_movie_blm_load_all;
  movie_class->save      = b_movie_blm_save;
}

static void
b_movie_blm_init (BMovie *movie)
{
  movie->maxval = 1;
}

static gboolean
b_movie_blm_load_info (BMovie      *movie,
                       GIOChannel  *io,
                       GError     **error)
{
  GString  *str   = g_string_new_len (NULL, 128);
  gboolean  magic = FALSE;

  while (g_io_channel_read_line_string (io, str, NULL,
                                        error) == G_IO_STATUS_NORMAL)
    {
      if (! b_movie_blm_parse_header_line (movie, str->str, str->len, &magic))
        {
          if (!magic)
            g_set_error (error, 0, 0, "Error parsing movie header.");
          break;
        }
    }

  g_string_free (str, TRUE);

  return magic;
}

static gboolean
b_movie_blm_load_all (BMovie      *movie,
                      GIOChannel  *io,
                      GError     **error)
{
  GString  *str = g_string_new_len (NULL, 128);
  guchar   *data          = NULL;
  gboolean  header_parsed = FALSE;
  gboolean  magic         = FALSE;
  gint      line          = -1;
  gint      duration;
 
  while (g_io_channel_read_line_string (io, str, NULL,
                                        error) == G_IO_STATUS_NORMAL)
    {
      if (!header_parsed)
        {
          if (! b_movie_blm_parse_header_line (movie,
                                               str->str, str->len, &magic))
            {
              if (magic)
                header_parsed = TRUE;
              else
                break;

              movie->duration = 0;
            }
        }

      if (header_parsed)
        {
          const gchar *buf = str->str;
          gint         len = str->len;

          if (!data)
            data = g_malloc (movie->width * movie->height);

          /* skip empty lines and bufs */
          if (len == 0 || buf[0] == '#' || g_ascii_isspace (buf[0]))
            continue;

          if (line == -1)
            {
              if (buf[0] == '@')
                {
                  if (sscanf (buf+1, "%d", &duration) == 1 && duration > 0)
                    line = 0;

                  if (duration < B_MOVIE_MIN_DELAY)
                    {
                      g_printerr ("Frame with %d ms duration, "
                                  "using %d ms instead\n",
                                  duration, B_MOVIE_MIN_DELAY);
                      duration = B_MOVIE_MIN_DELAY;
                    }
                }
            }
          else
            {
              if (buf[0] == '@' || len - 1 < movie->width)
                {
                  g_printerr ("Invalid frame, skipping.\n");
                  line = -1;
                }
              else
                {
                  gint i;

                  for (i = 0; i < movie->width; i++)
                    {
                      gint d = (buf[i] == '1' ? 1 : 0);
                      data[movie->width * line + i] = d * movie->maxval;
                    }

                  if (++line == movie->height)
                    {
                      b_movie_prepend_frame (movie, duration, data);
                      line = -1;
                    }
                }            
            }
        }
    }
 
  g_free (data);
  g_string_free (str, TRUE);

  /* caller checks movie->n_frames and sets an error if no frames were found */

  return TRUE;
}

static gboolean
b_movie_blm_save (BMovie  *movie,
                  FILE    *stream,
                  GError **error)
{
  GList *list;

  if (movie->channels != 1)
    {
      g_set_error (error, 0, 0,
                   "Cannot save movie with more than one channel as BLM");
      return FALSE;
    }

  fprintf (stream,
           "# Blinkenlights Movie %dx%d\n", movie->width, movie->height);

  b_movie_blm_save_header (stream, "name",        movie->title);
  b_movie_blm_save_header (stream, "description", movie->description);
  b_movie_blm_save_header (stream, "creator",     movie->creator);
  b_movie_blm_save_header (stream, "creator",     "blib " VERSION);
  b_movie_blm_save_header (stream, "author",      movie->author);
  b_movie_blm_save_header (stream, "email",       movie->email);
  b_movie_blm_save_header (stream, "url",         movie->url);

  fprintf (stream, "# duration = %d\n", movie->duration); 

  for (list = movie->frames; list; list = list->next)
    {
      BMovieFrame *frame = list->data;
      const guchar *data = frame->data;
      gint x, y;

      fprintf (stream, "\n@%d\n", frame->duration);

      for (y = 0; y < movie->height; y++)
        {
          for (x = 0; x < movie->width; x++, data++)
            fputc ((*data > movie->maxval / 2) ? '1' : '0', stream);

          fputc ('\n', stream);
        }
    }

  return TRUE;
}

static gboolean
b_movie_blm_parse_header_line (BMovie      *movie,
                               const gchar *buf,
                               gsize        len,
                               gboolean    *magic)
{
  const gchar *comment;

  if (!len)
    return FALSE;

  if (! *magic)
    {
      gint width;
      gint height;

      if (sscanf (buf, " %dx%d", &width, &height) != 2 ||
          width < 1 || height < 1)
        {
          return FALSE;
        }

      movie->width  = width;
      movie->height = height;

      *magic = TRUE;
      return TRUE;
    }

  if (buf[0] != '#')
    return FALSE;

  comment = buf + 1;
  len--;
  while (len && g_ascii_isspace (*comment))
    {
      comment++;
      len--;
    }

  if (!g_utf8_validate (comment, -1, NULL))
    return TRUE;  /* ignore silently */

  if (!movie->title)
    {
      if (g_ascii_strncasecmp (comment, "name=", 5) == 0)
        movie->title = g_strstrip (g_strndup (comment + 5, len - 6));
      else if (g_ascii_strncasecmp (comment, "name =", 6) == 0)
        movie->title = g_strstrip (g_strndup (comment + 6, len - 7));
    }
  if (!movie->description)
    { 
      if (g_ascii_strncasecmp (comment, "description=", 12) == 0)
        movie->description = g_strstrip (g_strndup (comment + 12, len - 13));
      else if (g_ascii_strncasecmp (comment, "description =", 13) == 0)
        movie->description = g_strstrip (g_strndup (comment + 13, len - 14));
    }
  if (!movie->duration)
    {
      if (g_ascii_strncasecmp (comment, "duration=", 9) == 0 ||
          g_ascii_strncasecmp (comment, "duration =", 10) == 0)
        {
          sscanf (comment + 9, "%d", &movie->duration);
        }
    }

  return TRUE;
}

static void
b_movie_blm_save_header (FILE        *stream,
                         const gchar *name, 
                         const gchar *value)
{
  const gchar *end;

  if (!value || !*value)
    return;

  fprintf (stream, "# %s = ", name);
  
  end = strchr (value, '\n');
  if (!end)
    end = value + MIN (strlen (value), 70 - strlen (name));

  if (*end)
    {
      gchar *dup = g_strndup (value, end - value);
      fprintf (stream, dup);
      g_free (dup);
    }
  else
    {
      fprintf (stream, value);
    }

  fputc ('\n', stream);
}

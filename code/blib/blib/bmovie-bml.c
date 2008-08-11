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

#include <stdlib.h>

#include <glib-object.h>

#include "btypes.h"
#include "bobject.h"
#include "bmovie-bml.h"
#include "bmovie-bml-parser.h"
#include "bwriter.h"


static void       b_movie_bml_class_init (BMovieBMLClass  *class);

static gboolean   b_movie_bml_load_info  (BMovie          *movie,
                                          GIOChannel      *io,
                                          GError         **error);
static gboolean   b_movie_bml_load_all   (BMovie          *movie,
                                          GIOChannel      *io,
                                          GError         **error);
static gboolean   b_movie_bml_save       (BMovie          *movie,
                                          FILE            *stream,
                                          GError         **error);


static BMovieClass *parent_class = NULL;


GType
b_movie_bml_get_type (void)
{
  static GType movie_type = 0;

  if (!movie_type)
    {
      static const GTypeInfo movie_info =
      {
        sizeof (BMovieBMLClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_movie_bml_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BMovieBML),
        0,              /* n_preallocs */
        NULL            /* instance_init */
      };

      movie_type = g_type_register_static (B_TYPE_MOVIE,
                                           "BMovieBML", 
                                           &movie_info, 0);
    }

  return movie_type;
}

static void
b_movie_bml_class_init (BMovieBMLClass *class)
{
  BMovieClass *movie_class;

  parent_class = g_type_class_peek_parent (class);

  movie_class  = B_MOVIE_CLASS (class);

  movie_class->load_info = b_movie_bml_load_info;
  movie_class->load_all  = b_movie_bml_load_all;
  movie_class->save      = b_movie_bml_save;
}

static gboolean
b_movie_bml_load_info (BMovie      *movie,
                       GIOChannel  *io,
                       GError     **error)
{
  return b_movie_bml_parse_bml (movie, io, TRUE, error);
}

static gboolean
b_movie_bml_load_all (BMovie      *movie,
                      GIOChannel  *io,
                      GError     **error)
{
  return b_movie_bml_parse_bml (movie, io, FALSE, error);
}


static gboolean
b_movie_bml_save (BMovie  *movie,
                  FILE    *stream,
                  GError **error)
{
  BWriter *writer;
  GList   *list;
  gchar    width[16];
  gchar    height[16];
  gchar    buf[16];
  gchar   *row;
  guint    len, bits;

  if (movie->channels != 1)
    {
      g_set_error (error, 0, 0,
                   "Cannot yet save movie with more than one channel as BML");
      return FALSE;
    }

  /* calculate bits from movie->maxval */
  for (len = movie->maxval, bits = 0; len; len >>= 1, bits++)
    ;

  g_snprintf (width,  sizeof (width),  "%d", movie->width);
  g_snprintf (height, sizeof (height), "%d", movie->height);
  g_snprintf (buf,    sizeof (buf),    "%u", bits);

  writer = b_writer_new (stream, 4);

  b_write_header (writer, "utf-8");

  b_write_open_tag (writer, "blm",
                    "width", width, "height", height, "bits", buf, NULL);

  if (movie->title)
    {
      b_write_open_tag (writer, "header", NULL);

#define BML_WRITE_META(tag) \
      if (movie->tag) b_write_element (writer, #tag, movie->tag, NULL);

      BML_WRITE_META (title);
      BML_WRITE_META (description);

      if (movie->creator)
        b_write_element (writer, "creator", movie->creator, NULL);
      else
        b_write_element (writer, "creator", "blib " VERSION, NULL);

      BML_WRITE_META (author);
      BML_WRITE_META (email);
      BML_WRITE_META (url);
      
      g_snprintf (buf, sizeof (buf), "%d", movie->duration);
      b_write_element (writer, "duration", buf, NULL);

      if (movie->loop)
        b_write_element (writer, "loop", NULL, NULL);

#undef BML_WRITE_META

      b_write_close_tag (writer, "header");
    }

  /* calculate length of row data */
  len = movie->width * (bits > 4 ? 2 : 1);
  row = g_new (gchar, len + 1);
  row[len] = '\0';

  for (list = movie->frames; list; list = list->next)
    {
      BMovieFrame *frame = list->data;
      guchar      *data  = frame->data;
      gint         x, y;

      g_snprintf (buf, sizeof (buf), "%d", frame->duration);
      b_write_open_tag (writer, "frame", "duration", buf, NULL);

      for (y = 0; y < movie->height; y++)
        {
          if (bits > 4)
            for (x = 0; x < movie->width; x++, data++)
              g_snprintf (row + x * 2, 3, "%02x", *data);
          else
            for (x = 0; x < movie->width; x++, data++)
              g_snprintf (row + x, 2, "%1x", *data);
                        
          b_write_element (writer, "row", row, NULL);
        }

      b_write_close_tag (writer, "frame");
    }

  g_free (row);

  b_write_close_tag (writer, "blm");

  b_writer_free (writer);

  return TRUE;
}

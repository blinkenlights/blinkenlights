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

#include <glib-object.h>

#include "btypes.h"
#include "bmovie-gif.h"
#include "bmovie-gif-parser.h"

#include "gif-save.h"


static void       b_movie_gif_class_init (BMovieGIFClass  *class);
static void       b_movie_gif_init       (BMovie          *movie);

static gboolean   b_movie_gif_load_info  (BMovie          *movie,
                                          GIOChannel      *io,
                                          GError         **error);
static gboolean   b_movie_gif_load_all   (BMovie          *movie,
                                          GIOChannel      *io,
                                          GError         **error);
static gboolean   b_movie_gif_save       (BMovie          *movie,
                                          FILE            *stream,
                                          GError         **error);


static BMovieClass *parent_class = NULL;


GType
b_movie_gif_get_type (void)
{
  static GType movie_type = 0;

  if (!movie_type)
    {
      static const GTypeInfo movie_info =
      {
        sizeof (BMovieGIFClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_movie_gif_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BMovieGIF),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_movie_gif_init
      };

      movie_type = g_type_register_static (B_TYPE_MOVIE,
                                           "BMovieGIF", 
                                           &movie_info, 0);
    }
  
  return movie_type;
}

static void
b_movie_gif_class_init (BMovieGIFClass *class)
{
  BMovieClass *movie_class;

  parent_class = g_type_class_peek_parent (class);

  movie_class  = B_MOVIE_CLASS (class);

  movie_class->load_info = b_movie_gif_load_info;
  movie_class->load_all  = b_movie_gif_load_all;
  movie_class->save      = b_movie_gif_save;
}

static void
b_movie_gif_init (BMovie *movie)
{
  movie->maxval = 255;
}

static gboolean
b_movie_gif_load_info (BMovie      *movie,
                       GIOChannel  *io,
                       GError     **error)
{
  return b_movie_gif_parse_gif (movie, io, TRUE, error);
}

static gboolean
b_movie_gif_load_all (BMovie      *movie,
                      GIOChannel  *io,
                      GError     **error)
{
  return b_movie_gif_parse_gif (movie, io, FALSE, error);
}

static gboolean
b_movie_gif_save (BMovie  *movie,
                  FILE    *stream,
                  GError **error)
{
  GList  *list;
  guchar *cmap;
  guint   i, len;
  guint   bits;

  g_return_val_if_fail (movie->maxval > 0, FALSE);

  /* calculate bits from movie->maxval */
  for (len = movie->maxval, bits = 0; len; len >>= 1, bits++)
    ;

  len = 1 << bits;
  cmap = g_new0 (guchar, 3 * len);

  for (i = 0; i <= movie->maxval; i++)
    {
      guint value = (i * 255) / movie->maxval;

      cmap[i*3 + 0] = value;
      cmap[i*3 + 1] = value;
      cmap[i*3 + 2] = value;
    }

  GIFEncodeHeader (stream, TRUE, movie->width, movie->height, 0, bits, cmap);

  if (movie->loop)
    GIFEncodeLoopExt (stream, 0);

  GIFEncodeCommentExt (stream, "Blinkenlights Movie written by blib " VERSION);

  for (list = movie->frames; list; list = list->next)
    {
      BMovieFrame *frame = list->data;
      guchar      *data  = frame->data;

      GIFEncodeGraphicControlExt (stream,
                                  DISPOSE_REPLACE, frame->duration, TRUE, -1); 
      GIFEncodeImageData (stream,
                          movie->width, movie->height, bits, 0, 0, data);
    }

  GIFEncodeClose (stream); 

  g_free (cmap);

  return TRUE;
}

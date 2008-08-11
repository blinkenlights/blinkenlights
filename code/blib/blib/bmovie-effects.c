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
#include "beffects.h"
#include "bmovie.h"
#include "bmovie-effects.h"


/**
 * b_movie_normalize:
 * @movie: a #BMovie object
 * @maxval: the new maximum value to apply to @movie
 * 
 * Changes the maximum value for @movie, that is the value that will
 * cause the lamp to glow with full intensity. This function iterates
 * over all frames of the movie and changes the frame data accordingly.
 **/
void
b_movie_normalize (BMovie *movie,
                   gint    maxval)
{
  GList *list;

  g_return_if_fail (B_IS_MOVIE (movie));
  g_return_if_fail (maxval > 0 && maxval < 256);
  
  if (movie->maxval == maxval)
    return;

  for (list = movie->frames; list; list = list->next)
    {
      BMovieFrame *frame = list->data;
      guchar      *data  = frame->data;
      gint         x, y;

      for (y = 0; y < movie->height; y++)
        for (x = 0; x < movie->width; x++, data++)
          *data = ((gint) *data * maxval) / movie->maxval;
    }
  
  movie->maxval = maxval;
}

/**
 * b_movie_apply_colormap:
 * @movie: a #BMovie object
 * @map: a LUT that remaps each color value
 *
 * Applies a mapping defined by @map to all frames of the @movie. The
 * mapping must not exceed the movie's maxval.
 **/
void
b_movie_apply_colormap (BMovie *movie,
                        guchar *map)
{
  GList *list;

  g_return_if_fail (B_IS_MOVIE (movie));
  g_return_if_fail (map != NULL);

  for (list = movie->frames; list; list = list->next)
    {
      BMovieFrame *frame = list->data;
      guchar      *data  = frame->data;
      gint         x, y;

      for (y = 0; y < movie->height; y++)
        for (x = 0; x < movie->width; x++, data++)
          *data = map[*data];
    }
}

/**
 * b_movie_apply_effects:
 * @movie: a #BMovie object
 * @effects: a #BEffects object or %NULL
 * @reverse: whether to reverse the order of frames in @movie
 * @speed: an adjustment factor to be applied on the frame timing
 * 
 * If @effects is non-%NULL, the effects described by it are applied
 * to @movie. If @reverse is %TRUE, the order of frames is reversed.
 * If @speed is different from 1.0, the frame timing is adjusted; for
 * example a value of 2.0 makes the movie play with double speed.
 **/
void
b_movie_apply_effects (BMovie   *movie,
                       BEffects *effects,
                       gboolean  reverse,
                       gdouble   speed)
{
  GList *list;

  g_return_if_fail (B_IS_MOVIE (movie));
  g_return_if_fail (effects == NULL || B_IS_EFFECTS (effects));
  g_return_if_fail (speed > 0.0);

  for (list = movie->frames; list; list = list->next)
    {
      BMovieFrame *frame = list->data;

      if (effects)
        {
          b_effects_apply (effects,
                           frame->data,
                           movie->width, movie->height,
                           movie->channels, movie->maxval);
        }

      if (speed != 1.0)
        {
          frame->start    /= speed;
          frame->duration /= speed;
        }
    }

  if (speed != 1.0)
    {
      movie->duration /= speed;
    }

  if (reverse)
    {
      gint start = 0;

      movie->frames = g_list_reverse (movie->frames);

      for (list = movie->frames; list; list = list->next)
        {
          BMovieFrame *frame = list->data;

          frame->start = start;
          start += frame->duration;
        }
    }
}

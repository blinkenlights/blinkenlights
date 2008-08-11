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
#include "bmovie.h"
#include "bmovie-gif-parser.h"

#include "gif-load.h"

/*  #define DEBUG_GIF 1  */


static void
b_movie_gif_compose_frame (BMovie         *movie,
                           guchar         *gray_buf,
                           guchar         *image_buf,
                           gint            image_colors,
                           guchar         *image_cmap,
                           guchar         *frame_buf,
                           GIFDisposeType  frame_disposal,
                           gint            frame_duration,
                           gint            frame_transparent,
                           gint            frame_width,
                           gint            frame_height,
                           gint            frame_off_x,
                           gint            frame_off_y,
                           gint            frame_colors,
                           guchar         *frame_cmap)
{
  guchar *src;
  guchar *dest;
  guchar *d;
  guchar *cmap;
  gint    colors;
  gint    x, y;

#if DEBUG_GIF
  g_printerr ("GIF frame: "
              "%dx%d @ %d,%d disposal: %d duration: %d transp: %d cmap: %s\n",
              frame_width, frame_height,
              frame_off_x, frame_off_y,
              frame_disposal,
              frame_duration,
              frame_transparent,
              frame_cmap ? "yes" : "no");
#endif

  cmap   = frame_cmap ? frame_cmap   : image_cmap;
  colors = frame_cmap ? frame_colors : image_colors;

  src  = frame_buf;
  dest = image_buf + frame_off_y * movie->width + frame_off_x;

  for (y = 0; y < frame_height; y++)
    {
      d = dest;

      for (x = 0; x < frame_width; x++)
        {
          if (frame_transparent > -1    &&
              *src == frame_transparent &&
              frame_disposal != DISPOSE_REPLACE)
            {
              d++;
              src++;
            }
          else
            {
              *d++ = *src++;
            }
        }

      dest += movie->width;
    }

  src  = image_buf;
  dest = gray_buf;

  for (y = 0; y < movie->height; y++)
    {
      d = dest;

      for (x = 0; x < movie->width; x++)
        {
          if (*src < colors)
            {
              guchar *cmap_entry = cmap + *src * 3;

#define INTENSITY_RED   0.30
#define INTENSITY_GREEN 0.59
#define INTENSITY_BLUE  0.11
#define INTENSITY(r,g,b) ((r) * INTENSITY_RED   + \
                          (g) * INTENSITY_GREEN + \
                          (b) * INTENSITY_BLUE  + \
                          (1) * (1.0 / 256.0))

              *d = INTENSITY (cmap_entry[0], cmap_entry[1], cmap_entry[2]);

#undef INTENSITY_RED
#undef INTENSITS_GREEN
#undef INTENSITY_BLUE
#undef INTENSITY
            }
          else
            {
              *d = 0;
            }

          d++;
          src++;
        }

      dest += movie->width;
    }

  b_movie_prepend_frame (movie, frame_duration, gray_buf);
}

gboolean
b_movie_gif_parse_gif (BMovie      *movie,
                       GIOChannel  *io,
                       gboolean     lazy,
                       GError     **error)
{
  gint           width;
  gint           height;
  gint           background;
  gint           colors;
  guchar        *cmap         = NULL;
  GIFRecordType  record_type;
  gint           duration     = 0;
  gchar         *comment      = NULL;
  guchar        *buffer       = NULL;
  guchar        *frame_buffer = NULL;
  guchar        *gray_buffer  = NULL;
  gboolean       success      = FALSE;
  GIFDisposeType frame_disposal    = DISPOSE_REPLACE;
  gint           frame_delay       = 100;
  gint           frame_transparent = -1;

  if (! GIFDecodeHeader (io,
                         TRUE, &width, &height, &background, &colors, &cmap))
    goto cleanup;

  movie->width  = width;
  movie->height = height;

#if DEBUG_GIF
  g_printerr ("GIF background: %d colors: %d\n", background, colors);
#endif

  buffer       = g_new0 (guchar, width * height);
  frame_buffer = g_new0 (guchar, width * height);
  gray_buffer  = g_new0 (guchar, width * height);

  if (background > -1)
    memset (buffer, background, width * height);

  while (GIFDecodeRecordType (io, &record_type))
    {
      switch (record_type)
        {
        case IMAGE:
          {
            gint    frame_width;
            gint    frame_height;
            gint    frame_off_x;
            gint    frame_off_y;
            gint    frame_colors;
            guchar *frame_cmap;

            if (! GIFDecodeImage (io,
                                  &frame_width, &frame_height,
                                  &frame_off_x, &frame_off_y,
                                  &frame_colors, &frame_cmap,
                                  frame_buffer))
              {
                g_set_error (error, 0, 0,
                             "Broken or missing image frame");
                goto cleanup;
              }

            if (frame_delay < B_MOVIE_MIN_DELAY)
              {
                g_printerr ("Frame with %d ms duration, using %d ms instead\n",
                            frame_delay, B_MOVIE_DEFAULT_DELAY);
                frame_delay = B_MOVIE_DEFAULT_DELAY;
              }

            duration += frame_delay;

            if (! lazy)
              {
                b_movie_gif_compose_frame (movie,
                                           gray_buffer,
                                           buffer,
                                           colors,
                                           cmap,
                                           frame_buffer,
                                           frame_disposal,
                                           frame_delay,
                                           frame_transparent,
                                           frame_width,
                                           frame_height,
                                           frame_off_x,
                                           frame_off_y,
                                           frame_colors,
                                           frame_cmap);
              }

            g_free (frame_cmap);
          }
          break;

        case GRAPHIC_CONTROL_EXTENSION:
          if (! GIFDecodeGraphicControlExt (io,
                                            &frame_disposal,
                                            &frame_delay,
                                            &frame_transparent))
            {
              g_set_error (error, 0, 0,
                           "Broken or missing graphic control extension");
              goto cleanup;
            }
          break;

        case COMMENT_EXTENSION:
          if (! GIFDecodeCommentExt (io, &comment))
            {
              g_set_error (error, 0, 0,
                           "Broken or missing comment extension");
              goto cleanup;
            }
          break;

        case UNKNOWN_EXTENSION:
          GIFDecodeUnknownExt (io);
          break;

        case TERMINATOR:
          success = TRUE;
          goto cleanup;
        }
    }

 cleanup:

  if (success && lazy)
    movie->duration = duration;

  g_free (cmap);
  g_free (buffer);
  g_free (frame_buffer);
  g_free (gray_buffer);
  g_free (comment);

  return success;
}

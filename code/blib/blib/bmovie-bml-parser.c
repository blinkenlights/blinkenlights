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
#include <string.h>

#include <glib-object.h>

#include "btypes.h"
#include "bmovie.h"
#include "bmovie-bml-parser.h"
#include "bparser.h"
#include "butils.h"

enum
{
  PARSER_IN_BLM = B_PARSER_STATE_USER,
  PARSER_IN_HEADER,
  PARSER_IN_TITLE,
  PARSER_IN_DESCRIPTION,
  PARSER_IN_CREATOR,
  PARSER_IN_AUTHOR,
  PARSER_IN_EMAIL,
  PARSER_IN_URL,
  PARSER_IN_DURATION,
  PARSER_IN_LOOP,
  PARSER_IN_FRAME,
  PARSER_IN_ROW,
  PARSER_FINISH,
};

typedef struct _ParserData ParserData;

struct _ParserData
{
  gint      bits;
  gint      channels;
  
  gint      frame_duration;
  guchar   *frame_data;
  gint      frame_next_row;

  BMovie   *movie;
  gboolean  lazy;
};

static BParserState  parser_start_element   (BParserState   state,
                                             const gchar   *element_name,
                                             const gchar  **attribute_names,
                                             const gchar  **attribute_values,
                                             gpointer       user_data,
                                             GError       **error);
static BParserState  parser_end_element     (BParserState   state,
                                             const gchar   *element_name,
                                             const gchar   *cdata,
                                             gsize          cdata_len,
                                             gpointer       user_data,
                                             GError       **error);

static gboolean      parse_blm_attributes   (ParserData    *data,
                                             const gchar  **names,
                                             const gchar  **values);
static gboolean      parse_frame_attributes (ParserData    *data,
                                             const gchar  **names,
                                             const gchar  **values);


gboolean
b_movie_bml_parse_bml (BMovie      *movie,
                       GIOChannel  *io,
                       gboolean     lazy,
                       GError     **error)
{
  BParser      *parser;
  ParserData    data;
  gboolean      retval;

  data.movie      = movie;
  data.lazy       = lazy;
  data.frame_data = NULL;

  parser = b_parser_new (parser_start_element, parser_end_element, &data);

  retval = b_parser_parse_io_channel (parser, io, FALSE, error);

  if (retval && b_parser_get_state (parser) != PARSER_FINISH)
    {
      g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                   "This doesn't look like Blinkenlights Markup Language");
      retval = FALSE;
    }

  b_parser_free (parser);

  g_free (data.frame_data);

  return retval;
}


/* parser functions */

static BParserState
parser_start_element (BParserState   state,
                      const gchar   *element_name,
                      const gchar  **attribute_names,
                      const gchar  **attribute_values,
                      gpointer       user_data,
                      GError       **error)
{
  ParserData *data = (ParserData *) user_data;

  switch (state)
    {
    case B_PARSER_STATE_TOPLEVEL:
      if (! strcmp (element_name, "blm"))
        {
          if (! parse_blm_attributes (data, attribute_names, attribute_values))
            {
              g_set_error (error,
                           G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                           "Invalid attributes for blm element");
              break;
            }
          
          return PARSER_IN_BLM;
        }
      break;

    case PARSER_IN_BLM:
      if (! strcmp (element_name, "header"))
        return PARSER_IN_HEADER;

      if (data->lazy)
        return B_PARSER_STATE_UNKNOWN;

      if (! strcmp (element_name, "frame"))
        {
          if (! parse_frame_attributes (data,
                                        attribute_names, attribute_values))
            {
              g_set_error (error, G_MARKUP_ERROR,
                           G_MARKUP_ERROR_INVALID_CONTENT,
                           "Invalid attributes for frame element number %d",
                           data->movie->n_frames);
              break;
            }

          if (data->frame_data)
            memset (data->frame_data, 0,
                    data->movie->width * data->movie->height);
          else
            data->frame_data = g_new0 (guchar, (data->movie->width *
                                                data->movie->height));

          data->frame_next_row = 0;

          return PARSER_IN_FRAME;
        }
      break;

    case PARSER_IN_HEADER:
      if (! strcmp (element_name, "title"))
        return PARSER_IN_TITLE;

      if (! strcmp (element_name, "description"))
        return PARSER_IN_DESCRIPTION;

      if (! strcmp (element_name, "creator"))
        return PARSER_IN_CREATOR;

      if (! strcmp (element_name, "author"))
        return PARSER_IN_AUTHOR;

      if (! strcmp (element_name, "email"))
        return PARSER_IN_EMAIL;

      if (! strcmp (element_name, "url"))
        return PARSER_IN_URL;

      /* only parse duration if we are lazy-loading ! */
      if (! strcmp (element_name, "duration") && data->lazy)
        return PARSER_IN_DURATION;
      
      if (! strcmp (element_name, "loop"))
        return PARSER_IN_LOOP;

      break;

    case PARSER_IN_FRAME:
      if (! strcmp (element_name, "row"))
        {
          if (data->frame_next_row == data->movie->height)
            {
              g_set_error (error, G_MARKUP_ERROR,
                           G_MARKUP_ERROR_INVALID_CONTENT,
                           "Too many rows in frame number %d",
                           data->movie->n_frames);
              break;
            }

          return PARSER_IN_ROW;
        }
      break;

    default:
      break;
    }

  return B_PARSER_STATE_UNKNOWN;
}

static BParserState
parser_end_element (BParserState   state,
                    const gchar   *element_name,
                    const gchar   *cdata,
                    gsize          cdata_len,
                    gpointer       user_data,
                    GError       **error)
{
  ParserData *data = (ParserData *) user_data;

  switch (state)
    {
    case PARSER_IN_BLM:
      return PARSER_FINISH;

    case PARSER_IN_HEADER:
      return PARSER_IN_BLM;

    case PARSER_IN_TITLE:
      if (!data->movie->title)
        data->movie->title = g_strdup (cdata);
      return PARSER_IN_HEADER;

    case PARSER_IN_DESCRIPTION:
      if (!data->movie->description)
        data->movie->description = g_strdup (cdata);
      return PARSER_IN_HEADER;

    case PARSER_IN_CREATOR:
      if (!data->movie->creator)
        data->movie->creator = g_strdup (cdata);
      return PARSER_IN_HEADER;

    case PARSER_IN_AUTHOR:
      if (!data->movie->author)
        data->movie->author = g_strdup (cdata);
      return PARSER_IN_HEADER;

    case PARSER_IN_EMAIL:
      if (!data->movie->email)
        data->movie->email = g_strdup (cdata);
      return PARSER_IN_HEADER;

    case PARSER_IN_URL:
      if (!data->movie->url)
        data->movie->url = g_strdup (cdata);
      return PARSER_IN_HEADER;

    case PARSER_IN_DURATION:
      b_parse_int (cdata, & data->movie->duration);
      return PARSER_IN_HEADER;

    case PARSER_IN_LOOP:
      /* for backward compat, ignore loop if it has a value of "no" */
      if (!cdata_len || g_ascii_tolower (*cdata) != 'n')
        data->movie->loop = TRUE;
      return PARSER_IN_HEADER;
      
    case PARSER_IN_FRAME:
      if (data->frame_next_row != data->movie->height)
        {
          g_set_error (error,
                       G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                       "Too few rows in frame number %d",
                       data->movie->n_frames);
          break;
        }
      else
        {
          b_movie_prepend_frame (data->movie,
                                 data->frame_duration, data->frame_data);
        }
      return PARSER_IN_BLM;

    case PARSER_IN_ROW:
      {
        gchar  *row;
        gchar  *src;
        guchar *dest;
        gint    bpp_src;
        gint    i, x = 0;

        row = g_strdup (cdata);

        if ((data->bits <= 4 &&
             cdata_len != data->movie->width * data->channels) ||
            (data->bits > 4 &&
             cdata_len != data->movie->width * data->channels * 2))
          {
            g_set_error (error,
                         G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                         "Invalid row length in frame number %d",
                         data->movie->n_frames);
            goto row_done;
          }

        bpp_src = data->bits <= 4 ? 1 : 2;

        src  = row;
        dest = data->frame_data + (data->movie->width * data->frame_next_row);

        for (; x < data->movie->width; x++, src += bpp_src, dest += 1)
          {
            *dest = 0;

            for (i = 0; i < bpp_src; i++)        
              {
                *dest <<= 4;

                src[i] = g_ascii_tolower (src[i]);

                switch (src[i])
                  {
                  case '0':
                  case '1':
                  case '2':
                  case '3':
                  case '4':
                  case '5':
                  case '6':
                  case '7':
                  case '8':
                  case '9':
                    *dest += src[i] - '0';
                    break;

                  case 'a':
                  case 'b':
                  case 'c':
                  case 'd':
                  case 'e':
                  case 'f':
                    *dest += 10 + src[i] - 'a';
                    break;

                  default:
                    g_set_error (error,
                                 G_MARKUP_ERROR,
                                 G_MARKUP_ERROR_INVALID_CONTENT,
                                 "Invalid row data in frame number %d",
                                 data->movie->n_frames);
                    goto row_done;
                  }
              }

            if (*dest > data->movie->maxval)
              {
                g_set_error (error,
                             G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                             "Row data exceeds maxval (%d) in frame number %d",
                             data->movie->maxval, data->movie->n_frames);
                goto row_done;
              }
          }

      row_done:
        g_free (row);
        data->frame_next_row++;

        if (x != data->movie->width)
          return B_PARSER_STATE_UNKNOWN;
      }
      return PARSER_IN_FRAME;

    default:
      break;
    }

  return B_PARSER_STATE_UNKNOWN;
}

static gboolean
parse_blm_attributes (ParserData   *data,
                      const gchar **names,
                      const gchar **values)
{
  gint i;
  gint width    = 0;
  gint height   = 0;
  gint bits     = 1;
  gint channels = 1;

  for (i = 0; names[i] && values[i]; i++)
    {
      if (strcmp (names[i], "width") == 0)
        b_parse_int (values[i], &width);
      else if (strcmp (names[i], "height") == 0)
        b_parse_int (values[i], &height);
      else if (strcmp (names[i], "bits") == 0)
        b_parse_int (values[i], &bits);
      else if (strcmp (names[i], "channels") == 0)
        b_parse_int (values[i], &channels);
    }

  if (width > 0 && height > 0 &&
      (bits >= 1 && bits <= 8) && (channels == 1)) /* channels == 3 */
    {
      data->movie->width    = width;
      data->movie->height   = height;
      data->movie->maxval   = (1 << bits) - 1;
      data->movie->channels = channels;

      data->bits     = bits;
      data->channels = channels;

      return TRUE;
    }

  return FALSE;
}

static gboolean
parse_frame_attributes (ParserData   *data,
                        const gchar **names,
                        const gchar **values)
{
  gint i;
  gint duration = 0;

  for (i = 0; names[i] && values[i]; i++)
    {
      if (strcmp (names[i], "duration") == 0)
        b_parse_int (values[i], &duration);
    }

  if (duration < B_MOVIE_MIN_DELAY)
    {
      g_printerr ("Frame with %d ms duration, using %d ms instead\n",
                  duration, B_MOVIE_DEFAULT_DELAY);
      duration = B_MOVIE_DEFAULT_DELAY;
    }

  data->frame_duration = duration;

  return TRUE;
}

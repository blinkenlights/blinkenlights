/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (C) 2002  The Blinkenlights Crew
 *                     Sven Neumann <sven@gimp.org>
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
#include <stdlib.h>

#include <glib-object.h>

#include "btypes.h"
#include "bparser.h"


struct _BParser
{
  GMarkupParseContext *context;
  BParserState         state;
  BParserState         last_state;
  gint                 unknown_depth;
  GString             *cdata;
  gpointer             user_data;
  BParserStartFunc     start_element;
  BParserEndFunc       end_element;
};


static void    parser_start_element (GMarkupParseContext  *context,
                                     const gchar          *element_name,
                                     const gchar         **attribute_names,
                                     const gchar         **attribute_values,
                                     gpointer              user_data,
                                     GError              **error);
static void    parser_end_element   (GMarkupParseContext  *context,
                                     const gchar          *element_name,
                                     gpointer              user_data,
                                     GError              **error);
static void    parser_text          (GMarkupParseContext  *context,
                                     const gchar          *text,
                                     gsize                 text_len,
                                     gpointer              user_data,
                                     GError              **error);
static void    parser_start_unknown (BParser              *parser);
static void    parser_end_unknown   (BParser              *parser);


static const GMarkupParser markup_parser =
{
  parser_start_element,
  parser_end_element,
  parser_text,
  NULL, /* passthrough */
  NULL, /* error       */
};


/**
 * b_parser_new:
 * @start_element: the function to call when an element is started
 * @end_element: the function to call when an element is closed
 * @user_data: data to pass to the functions above
 *
 * Creates a new #BParser suited to parse XML files. The #BParser
 * should later be freed using b_parser_free().
 *
 * Return value: a newly allocated #BParser
 **/
BParser *
b_parser_new (BParserStartFunc  start_element,
              BParserEndFunc    end_element,
              gpointer          user_data)
{
  BParser *parser;

  parser = g_new0 (BParser, 1);

  parser->context = g_markup_parse_context_new (&markup_parser,
                                                0, parser, NULL);

  parser->state         = B_PARSER_STATE_TOPLEVEL;
  parser->cdata         = g_string_new (NULL);
  parser->user_data     = user_data;

  parser->start_element = start_element;
  parser->end_element   = end_element;

  return parser;
}

/**
 * b_parser_parse:
 * @parser: a #BParser
 * @text: pointer to a text buffer to parse
 * @text_len: the number of bytes to parse from @text
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Let the @parser process a chunk of @text. You need to call
 * b_parser_end_parse() after you passed the last chunk to the @parser.
 *
 * Return value: %TRUE if parsing was successful, %FALSE if an error occured
 **/
gboolean
b_parser_parse (BParser      *parser,
                const gchar  *text,
                gssize        text_len,
                GError      **error)
{
  g_return_val_if_fail (parser != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  return g_markup_parse_context_parse (parser->context, text, text_len, error);
}

/**
 * b_parser_end_parse:
 * @parser: a #BParser
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Finishes the @parser. After calling this function, you must not
 * call b_parser_parse() on the parser again.
 *
 * Return value: %TRUE if @parser was successfully finished, %FALSE
 * otherwise
 **/
gboolean
b_parser_end_parse (BParser  *parser,
                    GError  **error)
{
  g_return_val_if_fail (parser != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  return g_markup_parse_context_end_parse (parser->context, error);
}

/**
 * b_parser_parse_io_channel:
 * @parser: a #BParser
 * @io: a #GIOChannel to read the text to parse from
 * @recode: %TRUE if you want the parser to do automatic encoding conversion
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Reads data from the #GIOChannel @io and passes it to @parser. If
 * @recode is TRUE, the data should start with an XML header so this
 * function can determine the encoding of the XML data and convert it
 * to UTF-8 for you.
 *
 * Return value: %TRUE if parsing was successful, %FALSE otherwise
 **/
gboolean
b_parser_parse_io_channel (BParser     *parser,
                           GIOChannel  *io,
                           gboolean     recode,
                           GError     **error)
{
  GIOStatus  status;
  gchar      buffer[8192];
  gsize      len = 0;
  gsize      bytes;

  g_return_val_if_fail (parser != NULL, FALSE);
  g_return_val_if_fail (io != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (recode)
    {
      const gchar *io_encoding = g_io_channel_get_encoding (io);
      gchar       *encoding    = NULL;

      if (io_encoding && strcmp (io_encoding, "UTF-8"))
        {
          g_warning ("b_parser_parse_io_channel(): "
                     "The encoding has already been set on this IOChannel!");
          return FALSE;
        }

      /* try to determine the encoding */

      g_io_channel_set_encoding (io, NULL, NULL);

      while (len < sizeof (buffer) && !encoding)
        {
          status = g_io_channel_read_chars (io,
                                            buffer + len, 1, &bytes, error);
          len += bytes;

          if (status == G_IO_STATUS_ERROR)
            return FALSE;
          if (status == G_IO_STATUS_EOF)
            break;

          encoding = b_parse_encoding (buffer, len);
        }

      if (encoding)
        {
          if (!g_io_channel_set_encoding (io, encoding, error))
            return FALSE;

          g_free (encoding);
        }
      else
        {
          g_io_channel_set_encoding (io, "UTF-8", NULL);
        }
    }

  while (TRUE)
    {
      if (!b_parser_parse (parser, buffer, len, error))
        return FALSE;

      status = g_io_channel_read_chars (io,
                                        buffer, sizeof(buffer), &len, error);

      switch (status)
        {
        case G_IO_STATUS_ERROR:
          return FALSE;
        case G_IO_STATUS_EOF:
          return b_parser_end_parse (parser, error);
        case G_IO_STATUS_NORMAL:
        case G_IO_STATUS_AGAIN:
          break;
        }
    }
}

/**
 * b_parser_free:
 * @parser: a #BParser
 *
 * Frees the resources allocated for @parser. You must not access
 * @parser after calling this function.
 **/
void
b_parser_free (BParser *parser)
{
  g_return_if_fail (parser != NULL);

  g_markup_parse_context_free (parser->context);

  g_string_free (parser->cdata, TRUE);
  g_free (parser);
}

/**
 * b_parser_get_state:
 * @parser: a #BParser
 *
 * Retrieves the current state of @parser.
 *
 * Return value: the state of @parser
 **/
BParserState
b_parser_get_state (BParser *parser)
{
  g_return_val_if_fail (parser != NULL, B_PARSER_STATE_UNKNOWN);

  return parser->state;
}

static void
parser_start_element (GMarkupParseContext  *context,
                      const gchar          *element_name,
                      const gchar         **attribute_names,
                      const gchar         **attribute_values,
                      gpointer              user_data,
                      GError              **error)
{
  BParserState  new_state;
  BParser      *parser = (BParser *) user_data;

  switch (parser->state)
    {
    case B_PARSER_STATE_TOPLEVEL:
    default:
      if (parser->start_element &&
          (new_state = parser->start_element (parser->state,
                                              element_name,
                                              attribute_names,
                                              attribute_values,
                                              parser->user_data,
                                              error)))
        {
          parser->last_state = parser->state;
          parser->state      = new_state;
          break;
        }
      /* else fallthru */
    case B_PARSER_STATE_UNKNOWN:
      parser_start_unknown (parser);
      break;
    }

  g_string_truncate (parser->cdata, 0);
}

static void
parser_end_element (GMarkupParseContext  *context,
                    const gchar          *element_name,
                    gpointer              user_data,
                    GError              **error)
{
  BParser *parser = (BParser *) user_data;

  switch (parser->state)
    {
    case B_PARSER_STATE_TOPLEVEL:
      g_assert_not_reached ();
      break;

    default:
      if (parser->end_element)
        {
          gint len;

          /* strip trailing spaces */
          for (len = parser->cdata->len;
               len > 0 && g_ascii_isspace (parser->cdata->str[len-1]);
               len--)
            ; /* do nothing */

          g_string_truncate (parser->cdata, len);

          parser->state = parser->end_element (parser->state,
                                               element_name,
                                               parser->cdata->str,
                                               parser->cdata->len,
                                               parser->user_data,
                                               error);
          break;
        }
      /* else fallthru */
    case B_PARSER_STATE_UNKNOWN:
      parser_end_unknown (parser);
      break;
    }

  g_string_truncate (parser->cdata, 0);
}

static void
parser_text (GMarkupParseContext  *context,
             const gchar          *text,
             gsize                 text_len,
             gpointer              user_data,
             GError              **error)
{
  BParser   *parser = (BParser *) user_data;
  gboolean     space;
  gint         i;

  space = (parser->cdata->len == 0 ||
           g_ascii_isspace (parser->cdata->str[parser->cdata->len]));

  for (i = 0; i < text_len; i++)
    {
      if (g_ascii_isspace (text[i]))
        {
          if (space)
            continue;
          space = TRUE;
        }
      else
        {
          space = FALSE;
        }

      g_string_append_c (parser->cdata, text[i]);
    }
}

static void
parser_start_unknown (BParser *parser)
{
  if (parser->unknown_depth == 0)
    {
      parser->last_state = parser->state;
      parser->state = B_PARSER_STATE_UNKNOWN;
    }

  parser->unknown_depth++;
}

static void
parser_end_unknown (BParser *parser)
{
  parser->unknown_depth--;

  if (parser->unknown_depth == 0)
    parser->state = parser->last_state;
}

/**
 * b_parse_encoding:
 * @text: a string to parse, must be at least 20 bytes
 * @text_len: the maximum number of bytes to parse from @text
 *
 * Scans the @text for an XML header with encoding specification.
 *
 * Return value: a copy of the encoding string or %NULL if none was
 * found
 **/
gchar *
b_parse_encoding (const gchar *text,
                  gint         text_len)
{
  const gchar *start;
  const gchar *end;
  gint         i;

  g_return_val_if_fail (text, NULL);

  if (text_len < 20)
    return NULL;

  start = g_strstr_len (text, text_len, "<?xml");
  if (!start)
    return NULL;

  end = g_strstr_len (start, text_len - (start - text), "?>");
  if (!end)
    return NULL;

  text_len = end - start;
  if (text_len < 12)
    return NULL;

  start = g_strstr_len (start + 1, text_len - 1, "encoding=");
  if (!start)
    return NULL;

  start += 9;
  if (*start != '\"' && *start != '\'')
    return NULL;

  text_len = end - start;
  if (text_len < 1)
    return NULL;

  for (i = 1; i < text_len; i++)
    if (start[i] == start[0])
      break;

  if (i == text_len || i < 3)
    return NULL;

  return g_strndup (start + 1, i - 1);
}

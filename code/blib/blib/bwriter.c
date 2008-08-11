/* blib - Library of useful things to hack the Blinkenlights
 * 
 * Copyright (C) 2002  The Blinkenlights Crew
 *                     Sven Neumann <sven@gimp.org>
 * 
 * Based on code written 2001 for convergence integrated media GmbH.
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
#include "bwriter.h"

struct _BWriter
{
  FILE *stream;
  gint  indent;
  gint  indent_level;
};


static void        b_write_attributes  (BWriter *writer,
                                        va_list  attributes);
static void inline b_write_indent      (BWriter *writer);


/**
 * b_writer_new:
 * @stream: a FILE stream prepared for writing
 * @indent: how many characters to indent per nesting level
 * 
 * Creates a new #BWriter which gives a convenient interface to write
 * XML data to @stream. The writer should later be freed using
 * b_writer_free().
 * 
 * Return value: a newly allocate #BWriter 
 **/
BWriter *
b_writer_new (FILE *stream,
              gint  indent)
{
  BWriter *writer;

  g_return_val_if_fail (stream != NULL, NULL);
  g_return_val_if_fail (indent >= 0, NULL);

  writer = g_new0 (BWriter, 1);

  writer->stream = stream;
  writer->indent = indent;

  return writer;
}

/**
 * b_writer_free:
 * @writer: a #BWriter
 * 
 * Frees the resources allocated for @writer. You must not access
 * @writer after calling this function.
 **/
void
b_writer_free (BWriter *writer)
{
  g_return_if_fail (writer != NULL);

  g_free (writer);
}

/**
 * b_write_header:
 * @writer: a #BWriter
 * @encoding: an optional encoding string or %NULL
 * 
 * Writes an XML header to the stream associated with @writer.
 **/
void
b_write_header (BWriter     *writer,
                const gchar *encoding)
{
  g_return_if_fail (writer != NULL);

  if (encoding && *encoding)
    fprintf (writer->stream,
             "<?xml version=\"1.0\" encoding=\"%s\"?>\n", encoding);
  else
    fprintf (writer->stream, "<?xml version=\"1.0\"?>\n");
}

/**
 * b_write_open_tag:
 * @writer: a #BWriter
 * @tag: the name of the element to open
 * @Varargs: an optional key/value pairs interpreted as attributes
 * 
 * Writes an opening XML tag with the element name @tag to the stream
 * associated with @writer.  You can pass a %NULL-terminated list of
 * key/value pairs that are written out as attributes.
 **/
void
b_write_open_tag (BWriter     *writer,
                  const gchar *tag,
                  ...)
{
  va_list attributes;

  g_return_if_fail (writer != NULL);
  g_return_if_fail (tag != NULL);

  va_start (attributes, tag);

  b_write_indent (writer);

  fprintf (writer->stream, "<%s", tag);
  b_write_attributes (writer, attributes);
  fprintf (writer->stream, ">\n");

  writer->indent_level++;

  va_end (attributes);
}

/**
 * b_write_close_tag:
 * @writer: a #BWriter
 * @tag: the name of the element to close
 * 
 * Writes a closing XML tag with the element name @tag to the stream
 * associated with @writer.
 **/
void
b_write_close_tag (BWriter     *writer,
                   const gchar *tag)
{
  g_return_if_fail (writer != NULL);
  g_return_if_fail (tag != NULL);

  writer->indent_level--;
  b_write_indent (writer);

  fprintf (writer->stream, "</%s>\n", tag);
}

/**
 * b_write_element:
 * @writer: a #BWriter
 * @tag: the element name
 * @value: the element value
 * @Varargs: an optional key/value pairs interpreted as attributes
 * 
 * Writes an XML element with the name @tag and the value @value to
 * the stream associated with @writer. If @value is %NULL, an empty
 * element is written. You can pass a %NULL-terminated list of
 * key/value pairs that are written out as attributes.
 **/
void
b_write_element (BWriter     *writer,
                 const gchar *tag,
                 const gchar *value,
                 ...)
{
  va_list attributes;

  g_return_if_fail (writer != NULL);
  g_return_if_fail (tag != NULL);

  va_start (attributes, value);

  b_write_indent (writer);

  fprintf (writer->stream, "<%s", tag);
  b_write_attributes (writer, attributes);

  if (value)
    {
      gchar *escaped = g_markup_escape_text (value, strlen (value));
      fprintf (writer->stream, ">%s</%s>\n", escaped, tag);
      g_free (escaped);
    }
  else
    {
      fprintf (writer->stream, "/>\n");
    }

  va_end (attributes);
}


/*  private functions  */

static void
b_write_attributes (BWriter *writer,
                    va_list  attributes)
{
  const gchar *name;
  const gchar *attribute;

  name = va_arg (attributes, const gchar *);

  while (name)
    {
      attribute = va_arg (attributes, const gchar *);

      fprintf (writer->stream, " %s=\"%s\"", name, attribute);
      
      name = va_arg (attributes, const gchar *);
    }
}

static const gchar *spaces = "                ";  /* 16 spaces */

static inline void
b_write_indent (BWriter *writer)
{
  gint indent = writer->indent * writer->indent_level;

  while (indent > 16)
    {
      fprintf (writer->stream, spaces);
      indent -= 16;
    }
  fprintf (writer->stream, spaces + 16 - indent);
}

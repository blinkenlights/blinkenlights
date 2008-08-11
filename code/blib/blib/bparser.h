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

#ifndef __B_PARSER_H__
#define __B_PARSER_H__

G_BEGIN_DECLS

typedef enum
{
  B_PARSER_STATE_UNKNOWN,
  B_PARSER_STATE_TOPLEVEL,
  B_PARSER_STATE_USER = 0x10  /* first user state, use as many as you need */
} BParserState;


/*  Called for open tags <foo bar="baz">, returns the new state or
    B_PARSER_STATE_UNKNOWN if it couldn't handle the tag.  */
typedef BParserState (* BParserStartFunc) (BParserState   state,
                                           const gchar   *element_name,
                                           const gchar  **attribute_names,
                                           const gchar  **attribute_values, 
                                           gpointer       user_data,
                                           GError       **error);
  /*  Called for close tags </foo>, returns the new state.  */
typedef BParserState (* BParserEndFunc)   (BParserState   state,
                                           const gchar   *element_name,
                                           const gchar   *cdata,
                                           gsize          cdata_len,
                                           gpointer       user_data,
                                           GError       **error);



BParser *b_parser_new              (BParserStartFunc   start_element,
                                    BParserEndFunc     end_element,
                                    gpointer           user_data);
void     b_parser_free             (BParser           *parser);

/* chunk parsing API */
gboolean b_parser_parse            (BParser           *parser,
                                    const gchar       *text,
                                    gssize             text_len,
                                    GError           **error);
gboolean b_parser_end_parse        (BParser           *parser,
                                    GError           **error);

/* convenience function for IO channels */
gboolean b_parser_parse_io_channel (BParser           *parser,
                                    GIOChannel        *io,
                                    gboolean           recode,
                                    GError           **error);

BParserState b_parser_get_state    (BParser           *parser);

/* parses an XML header */
gchar *  b_parse_encoding          (const gchar       *text,
                                    gint               text_len);

G_END_DECLS

#endif  /* __B_PARSER_H__ */

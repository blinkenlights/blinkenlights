/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2002-2007  The Blinkenlights Crew
 *                          Stefan Schuermans <1stein@blinkenarea.org>
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

#ifndef __CHARACTERS_H__
#define __CHARACTERS_H__

#include <glib.h>

/* type for a font */
struct _ChFont
  {
    guint width, height,          /* parameters of the font */
          advance, line_advance;
    gchar *chars;                 /* the characters contained in this font
                                     (in same order as in data) */
    gchar *unknown;               /* font data for unknown charcter */
    gchar *data[];                /* font data for each character */
                                  /* (top to bottom, line-wise left to right, */
                                  /*  '0'=off, '1'=on) */
  };
typedef struct _ChFont ChFont;

/* no font */
extern const ChFont *const pChFontNone;

/* select a font */
/*   returns pointer to font (or pChFontNone if no suitable font is found) */
const ChFont *selectChFont (guint chars,       /* number of characters to display */
                                               /* in a line (for width comparison) */
                            guint lines,       /* number of lines to display */
                                               /* (for height comparison) */
                            guint max_width,   /* maximum width of chars characters */
                            guint max_height); /* maximum height of font */

/* get a character from a font */
/*   returns the character data as string */
/*     (top to bottom, line-wise left to right, '0'=off, '1'=on) */
const gchar * getChFontChar (const ChFont *font, /* the font to select the character from */
                             gchar chr);         /* the character to select */

#endif /*  __CHARACTERS_H__  */

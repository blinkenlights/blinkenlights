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

#ifndef __B_WRITER_H__
#define __B_WRITER_H__

#include <stdio.h>

G_BEGIN_DECLS


BWriter * b_writer_new      (FILE        *stream,
                             gint         indent);
void      b_writer_free     (BWriter     *writer);

void      b_write_header    (BWriter     *writer,
                             const gchar *encoding);
void      b_write_open_tag  (BWriter     *writer,
                             const gchar *tag,
                             ...);
void      b_write_close_tag (BWriter     *writer,
                             const gchar *tag);
void      b_write_element   (BWriter     *writer,
                             const gchar *tag,
                             const gchar *value,
                             ...);

G_END_DECLS

#endif  /* __B_WRITER_H__ */

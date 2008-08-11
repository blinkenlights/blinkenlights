/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (C) 2001-2003 The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
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

#ifndef __B_UTILS_H__
#define __B_UTILS_H__

gchar    * b_filename_from_utf8  (const gchar  *name,
                                  const gchar  *root,
                                  GError      **error);

gboolean   b_parse_int           (const gchar  *str,
                                  gint         *value);
gboolean   b_parse_boolean       (const gchar  *str,
                                  gboolean     *value);
gboolean   b_parse_double        (const gchar  *str,
                                  gdouble      *value);
gboolean   b_parse_coordinates   (const gchar **names,
                                  const gchar **values,
                                  gint         *x,
                                  gint         *y);
gboolean   b_parse_rectangle     (const gchar **names,
                                  const gchar **values,
                                  BRectangle   *rect);
gboolean   b_parse_color         (const gchar **names,
                                  const gchar **values,
                                  BColor       *color);
gboolean   b_parse_param         (GObject      *object,
                                  const gchar  *root,
                                  const gchar **names,
                                  const gchar **values,
                                  GError      **error);

gboolean   b_object_set_property (GObject      *object,
                                  const gchar  *key,
                                  const gchar  *value,
                                  const gchar  *root,
                                  GError      **error);

void       b_rectangle_union     (const BRectangle *src1,
                                  const BRectangle *src2,
                                  BRectangle       *dest);
gboolean   b_rectangle_intersect (const BRectangle *src1,
                                  const BRectangle *src2,
                                  BRectangle       *dest);

#endif /* __B_UTILS_H__ */

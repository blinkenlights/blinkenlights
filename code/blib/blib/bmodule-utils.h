/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
 *                          Daniel Mack <daniel@yoobay.net>
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

#ifndef __B_MODULE_UTILS_H__
#define __B_MODULE_UTILS_H__

G_BEGIN_DECLS

void  b_module_draw_point (BModule *module,
                           gint     x,
                           gint     y,
                           guchar   value);
void  b_module_draw_line  (BModule *module,
                           gint     x1,
                           gint     y1,
                           gint     x2,
                           gint     y2,
                           guchar   value);
void  b_module_fill       (BModule *module,
                           guchar   value);


G_END_DECLS

#endif /* __B_MODULE_H__ */

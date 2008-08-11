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

#ifndef __B_MOVIE_EFFECTS_H__
#define __B_MOVIE_EFFECTS_H__

G_BEGIN_DECLS

void      b_movie_normalize       (BMovie   *movie,
                                   gint      maxval);
void      b_movie_apply_colormap  (BMovie   *movie,
                                   guchar   *map);
void      b_movie_apply_effects   (BMovie   *movie,
                                   BEffects *effects,
                                   gboolean  reverse,
                                   gdouble   speed);

G_END_DECLS

#endif /* __B_MOVIE_EFFECTS_H__ */

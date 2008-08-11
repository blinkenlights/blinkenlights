/* b2mng
 * Creates MNG animations from Blinkenlights movies.
 *
 * Copyright (C) 2002-2004  Sven Neumann <sven@gimp.org>
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

#ifndef __B_MOVIE_MNG_H__
#define __B_MOVIE_MNG_H__

gboolean b_movie_save_as_mng (BMovie  *movie,
                              BTheme  *theme,
                              FILE    *stream,
                              gint     loops,
                              GError **error);

#endif /* __BLUTILS_H__  */

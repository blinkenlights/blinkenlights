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

#ifndef __B_MOVIE_BML_H__
#define __B_MOVIE_BML_H__

#include "bmovie.h"

G_BEGIN_DECLS

#define B_TYPE_MOVIE_BML            (b_movie_bml_get_type ())
#define B_MOVIE_BML(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_MOVIE_BML, BMovieBML))
#define B_MOVIE_BML_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_MOVIE_BML, BMovieBMLClass))
#define B_IS_MOVIE_BML(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_MOVIE_BML))
#define B_IS_MOVIE_BML_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_MOVIE_BML))

typedef struct _BMovieClass BMovieBMLClass;
typedef struct _BMovie      BMovieBML;

GType   b_movie_bml_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __B_MOVIE_BML_H__ */

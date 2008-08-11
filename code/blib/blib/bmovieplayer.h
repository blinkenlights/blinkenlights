/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2002  The Blinkenlights Crew
 *                     Michael Natterer <mitch@gimp.org>
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

#ifndef __B_MOVIE_PLAYER_H__
#define __B_MOVIE_PLAYER_H__

G_BEGIN_DECLS


#define B_TYPE_MOVIE_PLAYER            (b_movie_player_get_type ())
#define B_MOVIE_PLAYER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_MOVIE_PLAYER, BMoviePlayer))
#define B_MOVIE_PLAYER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_MOVIE_PLAYER, BMoviePlayerClass))
#define B_IS_MOVIE_PLAYER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_MOVIE_PLAYER))
#define B_IS_MOVIE_PLAYER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_MOVIE_PLAYER))
#define B_MOVIE_PLAYER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), B_TYPE_MOVIE_PLAYER, BMoviePlayerClass))

typedef struct _BMoviePlayerClass BMoviePlayerClass;

struct _BMoviePlayer
{
  BModule   parent_instance;

  BMovie   *movie;
  GList    *current;

  gchar    *filename;
  gboolean  reverse;

  gboolean  clear;
  gdouble   halign;
  gdouble   valign;
  gint      xoffset;
  gint      yoffset;
};

struct _BMoviePlayerClass
{
  BModuleClass  parent_class;

  void (* request_stop) (BModule *module);
};

GType b_movie_player_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __B_MOVIE_PLAYER_H__ */

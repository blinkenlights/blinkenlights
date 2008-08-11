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

#ifndef __B_MOVIE_H__
#define __B_MOVIE_H__

#include <stdio.h>

#include <blib/bobject.h>

G_BEGIN_DECLS

#define B_MOVIE_MIN_DELAY      20
#define B_MOVIE_DEFAULT_DELAY  100

#define B_TYPE_MOVIE            (b_movie_get_type ())
#define B_MOVIE(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_MOVIE, BMovie))
#define B_MOVIE_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_MOVIE, BMovieClass))
#define B_IS_MOVIE(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_MOVIE))
#define B_IS_MOVIE_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_MOVIE))
#define B_MOVIE_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), B_TYPE_MOVIE, BMovieClass))


typedef struct _BMovieClass BMovieClass;

struct _BMovieClass
{
  BObjectClass  parent_class;

  gboolean (* load_info) (BMovie      *movie,
                          GIOChannel  *io,
                          GError     **error);
  gboolean (* load_all)  (BMovie      *movie,
                          GIOChannel  *io,
                          GError     **error);
  gboolean (* save)      (BMovie      *movie,
                          FILE        *stream,
                          GError     **error);
};

struct _BMovie
{
  BObject       parent_instance;

  gint          width;
  gint          height;
  gint          channels;
  gint          maxval;

  /*  movie data  */
  gint          duration;
  gint          load_count;
  gint          n_frames;
  GList        *frames;

  /*  meta data  */
  gchar        *title;
  gchar        *description;
  gchar        *creator;
  gchar        *author;
  gchar        *email;
  gchar        *url;
  gboolean      loop;
};


typedef struct _BMovieFrame BMovieFrame;

struct _BMovieFrame
{
  gint          start;
  gint          duration;
  guchar       *data;
};


GType     b_movie_get_type          (void) G_GNUC_CONST;
BMovie  * b_movie_new_from_file     (const gchar  *filename,
                                     gboolean      lazy_load,
                                     GError      **error);
BMovie  * b_movie_new_from_fd       (gint          fd,
                                     GError      **error);

gboolean  b_movie_load              (BMovie       *movie,
                                     GError      **error);
void      b_movie_unload            (BMovie       *movie);

gboolean  b_movie_save              (BMovie       *movie,
                                     FILE         *stream,
                                     GError      **error);
gboolean  b_movie_save_as           (BMovie       *movie,
                                     GType         movie_type,
                                     FILE         *stream,
                                     GError      **error);

GList   * b_movie_get_frame_at_time (BMovie       *movie,
                                     GList        *seed,
                                     gint          time);

/*  private, should only be used by movie loaders  */
void      b_movie_prepend_frame     (BMovie       *movie, 
                                     gint          duration, 
                                     const guchar *data);


G_END_DECLS

#endif /* __B_MOVIE_H__ */

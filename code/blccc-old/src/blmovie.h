/* blccc - BlinkenLigths Chaos Control Center
 *
 * Copyright (c) 2001  Sven Neumann <sven@gimp.org>
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

#ifndef __BL_MOVIE_H__
#define __BL_MOVIE_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BL_TYPE_MOVIE            (bl_movie_get_type ())
#define BL_MOVIE(obj)            (GTK_CHECK_CAST ((obj), BL_TYPE_MOVIE, BlMovie))
#define BL_MOVIE_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BL_TYPE_MOVIE, BlMovieClass))
#define BL_IS_MOVIE(obj)         (GTK_CHECK_TYPE ((obj), BL_TYPE_MOVIE))
#define BL_IS_MOVIE_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BL_TYPE_MOVIE))

typedef struct _BlMovieClass  BlMovieClass;

struct _BlMovieClass
{
  GtkObjectClass  parent_class;
};

struct _BlMovie
{
  GtkObject       parent_instance;
  
  gchar          *filename;
  gchar          *name;
  gchar          *description;
  gint            duration;
  time_t          mtime;
  gboolean        loaded;
  gint            width;
  gint            height;
  gint            n_frames;
  GList          *frames;
};

struct _BlMovieFrame
{
  gint            start;
  guchar         *data;
};


GtkType    bl_movie_get_type          (void);
BlMovie  * bl_movie_new               (const gchar *filename);
gboolean   bl_movie_load              (BlMovie     *movie);
GList    * bl_movie_get_frame_at_time (BlMovie     *movie,
                                       GList       *seed,
                                       gint         time);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BL_MOVIE_H__ */

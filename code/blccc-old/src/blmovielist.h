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

#ifndef __BL_MOVIE_LIST_H__
#define __BL_MOVIE_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define BL_TYPE_MOVIE_LIST            (bl_movie_list_get_type ())
#define BL_MOVIE_LIST(obj)            (GTK_CHECK_CAST ((obj), BL_TYPE_MOVIE_LIST, BlMovieList))
#define BL_MOVIE_LIST_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BL_TYPE_MOVIE_LIST, BlMovieListClass))
#define BL_IS_MOVIE_LIST(obj)         (GTK_CHECK_TYPE ((obj), BL_TYPE_MOVIE_LIST))
#define BL_IS_MOVIE_LIST_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BL_TYPE_MOVIE_LIST))

typedef struct _BlMovieListClass  BlMovieListClass;

struct _BlMovieListClass
{
  GtkObjectClass  parent_class;
};

struct _BlMovieList
{
  GtkObject       parent_instance;
  
  gchar          *dirname;
  GList          *movies;
};


GtkType        bl_movie_list_get_type        (void);
BlMovieList  * bl_movie_list_new             (const gchar *dirname);
void           bl_movie_list_reload          (BlMovieList *list);
gint           bl_movie_list_get_num_movies  (BlMovieList *list);
void           bl_movie_list_foreach_movie   (BlMovieList *list,
                                              GFunc        func,
                                              gpointer     data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BL_MOVIE_LIST_H__ */

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

#ifndef __BL_MOVIE_VIEW_H__
#define __BL_MOVIE_VIEW_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BL_TYPE_MOVIE_VIEW            (bl_movie_view_get_type ())
#define BL_MOVIE_VIEW(obj)            (GTK_CHECK_CAST ((obj), BL_TYPE_MOVIE_VIEW, BlMovieView))
#define BL_MOVIE_VIEW_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BL_TYPE_MOVIE_VIEW, BlMovieViewClass))
#define BL_IS_MOVIE_VIEW(obj)         (GTK_CHECK_TYPE ((obj), BL_TYPE_MOVIE_VIEW))
#define BL_IS_MOVIE_VIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BL_TYPE_MOVIE_VIEW))

typedef struct _BlMovieViewClass  BlMovieViewClass;

struct _BlMovieViewClass
{
  GtkVBoxClass    parent_class;
};

struct _BlMovieView
{
  GtkVBox         parent_instance;

  GtkAdjustment  *adjustment;
  GtkWidget      *toggle;
  GtkWidget      *slider;
  GtkWidget      *name;
  GtkWidget      *description;

  GList          *current;

  gint            timeout;
  guint           timeout_id;

  BlPreview      *preview;
  BlMovie        *movie;
};


GtkType     bl_movie_view_get_type   (void);
GtkWidget * bl_movie_view_new        (BlPreview   *preview);
void        bl_movie_view_set_movie  (BlMovieView *view,
                                      BlMovie     *movie);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BL_MOVIE_VIEW_H__ */

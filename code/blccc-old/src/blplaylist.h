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

#ifndef __BL_PLAY_LIST_H__
#define __BL_PLAY_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define BL_TYPE_PLAY_LIST            (bl_play_list_get_type ())
#define BL_PLAY_LIST(obj)            (GTK_CHECK_CAST ((obj), BL_TYPE_PLAY_LIST, BlPlayList))
#define BL_PLAY_LIST_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BL_TYPE_PLAY_LIST, BlPlayListClass))
#define BL_IS_PLAY_LIST(obj)         (GTK_CHECK_TYPE ((obj), BL_TYPE_PLAY_LIST))
#define BL_IS_PLAY_LIST_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BL_TYPE_PLAY_LIST))

typedef struct _BlPlayListClass  BlPlayListClass;

struct _BlPlayListClass
{
  GtkCListClass  parent_class;
};

struct _BlPlayList
{
  GtkCList       parent_instance;
};


GtkType      bl_play_list_get_type        (void);
GtkWidget  * bl_play_list_new             (void);
void         bl_play_list_append_movie    (BlPlayList *playlist,
                                           BlMovie    *movie); 
void         bl_play_list_remove_selected (BlPlayList *playlist);
BlMovie    * bl_play_list_get_next_movie  (BlPlayList *playlist);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BL_PLAY_LIST_H__ */

/* blccc - Blinkenlights Chaos Control Center
 *
 * Copyright (c) 2001-2002  Sven Neumann <sven@gimp.org>
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

#ifndef __BL_PLAYLIST_H__
#define __BL_PLAYLIST_H__

G_BEGIN_DECLS

#include "blplaylistitem.h"


#define BL_TYPE_PLAYLIST            (bl_playlist_get_type ())
#define BL_PLAYLIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_PLAYLIST, BlPlaylist))
#define BL_PLAYLIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_PLAYLIST, BlPlaylistClass))
#define BL_IS_PLAYLIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_PLAYLIST))
#define BL_IS_PLAYLIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BL_TYPE_PLAYLIST))


typedef struct _BlPlaylistClass  BlPlaylistClass;

struct _BlPlaylist
{
  BlPlaylistItem  parent_instance;

  BlConfig       *config;

  gboolean        shuffle;

  guchar         *buffer;

  GList          *items;
  GList          *current;
};

struct _BlPlaylistClass
{
  BlPlaylistItemClass  parent_class;
};


GType            bl_playlist_get_type       (void) G_GNUC_CONST;
BlPlaylist     * bl_playlist_new            (BlConfig             *config);
BlPlaylist     * bl_playlist_new_from_file  (const gchar          *filename,
                                             BlConfig             *config,
                                             BModulePaintCallback  paint_callback,
                                             gpointer              paint_data);
gint             bl_playlist_length         (BlPlaylist           *playlist);
BlPlaylistItem * bl_playlist_load_next_item (BlPlaylist           *playlist,
                                             gboolean              wrap_around);

#if 0
BlPlaylistItem * bl_playlist_peek_next_item (BlPlaylist           *playlist,
                                             gboolean              wrap_around);
#endif


G_END_DECLS

#endif /* __BL_PLAYLIST_H__ */

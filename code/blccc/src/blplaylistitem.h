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

#ifndef __BL_PLAYLIST_ITEM_H__
#define __BL_PLAYLIST_ITEM_H__

G_BEGIN_DECLS


#define BL_TYPE_PLAYLIST_ITEM            (bl_playlist_item_get_type ())
#define BL_PLAYLIST_ITEM(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_PLAYLIST_ITEM, BlPlaylistItem))
#define BL_PLAYLIST_ITEM_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_PLAYLIST_ITEM, BlPlaylistItemClass))
#define BL_IS_PLAYLIST_ITEM(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_PLAYLIST_ITEM))
#define BL_IS_PLAYLIST_ITEM_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BL_TYPE_PLAYLIST_ITEM))
#define BL_PLAYLIST_ITEM_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BL_TYPE_PLAYLIST_ITEM, BlPlaylistItemClass))


typedef struct _BlPlaylistItemClass  BlPlaylistItemClass;

struct _BLPlaylistItem
{
  BObject    parent_instance;

  BModule   *module;
  BEffects  *effects;
  gint       count;
  gint       loop;
  gboolean   reverse;
  gdouble    speed;
  gdouble    likely;
};

struct _BlPlaylistItemClass
{
  BObjectClass  parent_class;

  /* virtual functions */
  gchar * (* describe)      (BlPlaylistItem *item);
  void    (* apply_effects) (BlPlaylistItem *item,
                             BEffects       *effects,
                             gint            loop,
                             gboolean        reverse,
                             gdouble         speed);
};


GType            bl_playlist_item_get_type      (void) G_GNUC_CONST;
BlPlaylistItem * bl_playlist_item_new           (void);

gchar          * bl_playlist_item_describe      (BlPlaylistItem *item);
void             bl_playlist_item_apply_effects (BlPlaylistItem *item,
                                                 BEffects       *effects,
                                                 gint            loop,
                                                 gboolean        reverse,
                                                 gdouble         speed);


G_END_DECLS

#endif /* __BL_PLAYLIST_ITEM_H__ */

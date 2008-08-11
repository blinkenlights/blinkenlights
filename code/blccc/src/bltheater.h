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

#ifndef __BL_THEATER_H__
#define __BL_THEATER_H__

G_BEGIN_DECLS

#define BL_TYPE_THEATER            (bl_theater_get_type ())
#define BL_THEATER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_THEATER, BlTheater))
#define BL_THEATER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_THEATER, BlTheaterClass))
#define BL_IS_THEATER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_THEATER))
#define BL_IS_THEATER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BL_TYPE_THEATER))


typedef struct _BlTheaterClass  BlTheaterClass;

struct _BlTheaterClass
{
  GObjectClass   parent_class;

  void (* item_finished) (BlTheater      *theater,
                          BlPlaylistItem *item,
                          gboolean        pushed);
};

struct _BlTheater
{
  GObject         parent_instance;

  gint            width;
  gint            height;

  guint           stop_signal_id;

  BlPlaylistItem *item;
  GList          *item_stack;

  BEffects       *effects;
  BSender        *sender;
  guchar         *item_buffer;
  guchar         *frame_buffer;

  BlLogger       *logger;
};


GType      bl_theater_get_type         (void) G_GNUC_CONST;
BlTheater *bl_theater_new              (BlConfig        *config);
gboolean   bl_theater_add_recipient    (BlTheater       *theater,
                                        const gchar     *hostname,
                                        GError         **error);
gboolean   bl_theater_remove_recipient (BlTheater       *theater,
                                        const gchar     *hostname,
                                        GError         **error);
void       bl_theater_set_effects      (BlTheater       *theater,
                                        BEffects        *effects);
void       bl_theater_set_item         (BlTheater       *theater,
                                        BlPlaylistItem  *item);
void       bl_theater_push_item        (BlTheater       *theater,
                                        BlPlaylistItem  *item);
void       bl_theater_pop_item         (BlTheater       *theater);
void       bl_theater_play             (BlTheater       *theater);
void       bl_theater_pause            (BlTheater       *theater);
void       bl_theater_finish           (BlTheater       *theater);
void       bl_theater_kill             (BlTheater       *theater);
gboolean   bl_theater_is_playing       (BlTheater       *theater);
void       bl_theater_event            (BlTheater       *theater,
                                        BModuleEvent    *event);
void       bl_theater_set_frame_data   (BlTheater       *theater,
                                        const guchar    *data);
gboolean   bl_theater_paint_callback   (BModule         *module,
                                        guchar          *buffer,
                                        gpointer         data);


G_END_DECLS

#endif /* __BL_THEATER_H__ */

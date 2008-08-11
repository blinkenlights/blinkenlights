/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2004  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
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

#ifndef __B_VIEW_PIXBUF_H__
#define __B_VIEW_PIXBUF_H__

G_BEGIN_DECLS

#define B_TYPE_VIEW_PIXBUF         (b_view_pixbuf_get_type ())
#define B_VIEW_PIXBUF(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_VIEW_PIXBUF, BViewPixbuf))
#define B_VIEW_PIXBUF_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_VIEW_PIXBUF, BViewPixbufClass))
#define B_IS_VIEW_PIXBUF(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_VIEW_PIXBUF))


typedef struct _BViewPixbufClass BViewPixbufClass;
typedef struct _BViewPixbuf      BViewPixbuf;

struct _BViewPixbufClass
{
  GObjectClass  parent_class;
};

struct _BViewPixbuf
{
  /*< private >*/
  GObject       parent_instance;

  BTheme       *theme;
  gint          rows;
  gint          columns;
  gint          channels;

  GdkPixbuf    *pixbuf;
  GHashTable   *images;
};


GType             b_view_pixbuf_get_type   (void) G_GNUC_CONST;
BViewPixbuf     * b_view_pixbuf_new        (gint               rows,
                                            gint               columns,
                                            gint               channels,
                                            GError           **error);
BViewPixbuf     * b_view_pixbuf_new_theme  (BTheme            *theme,
                                            gboolean           image_preload,
                                            GError           **error);

/* the view expects data with a maxval of 255 */
const GdkPixbuf * b_view_pixbuf_render     (BViewPixbuf       *view,
                                            const guchar      *frame_data,
                                            const BRectangle  *clip,
                                            GError           **error);

const GdkPixbuf * b_view_pixbuf_load_image (BViewPixbuf       *view,
                                            const gchar       *filename,
                                            GError           **error);

G_END_DECLS

#endif /* __B_VIEW_PIXBUF_H__ */

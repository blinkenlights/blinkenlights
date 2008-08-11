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

#ifndef __B_VIEW_GTK_H__
#define __B_VIEW_GTK_H__

G_BEGIN_DECLS

#define B_TYPE_VIEW_GTK         (b_view_gtk_get_type ())
#define B_VIEW_GTK(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_VIEW_GTK, BViewGtk))
#define B_VIEW_GTK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_VIEW_GTK, BViewGtkClass))
#define B_IS_VIEW_GTK(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_VIEW_GTK))


typedef struct _BViewGtkClass BViewGtkClass;
typedef struct _BViewGtk      BViewGtk;

struct _BViewGtkClass
{
  GtkDrawingAreaClass  parent_class;
};

struct _BViewGtk
{
  /*< private >*/
  GtkDrawingArea       parent_instance;

  BViewPixbuf         *pixview;
  BTheme              *theme;
  gboolean             blend;
  guchar              *frame_data;

  GHashTable          *drawables;
};


GType       b_view_gtk_get_type (void) G_GNUC_CONST;
GtkWidget * b_view_gtk_new      (BTheme        *theme,
                                 GError       **error);

/* the view expects data with a maxval of 255 */
void        b_view_gtk_update   (BViewGtk      *view,
                                 const guchar  *frame_data);

G_END_DECLS

#endif /* __B_VIEW_GTK_H__ */

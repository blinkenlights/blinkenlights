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

#ifndef __BL_PREVIEW_H__
#define __BL_PREVIEW_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */


#define BL_TYPE_PREVIEW            (bl_preview_get_type ())
#define BL_PREVIEW(obj)            (GTK_CHECK_CAST ((obj), BL_TYPE_PREVIEW, BlPreview))
#define BL_PREVIEW_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BL_TYPE_PREVIEW, BlPreviewClass))
#define BL_IS_PREVIEW(obj)         (GTK_CHECK_TYPE ((obj), BL_TYPE_PREVIEW))
#define BL_IS_PREVIEW_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BL_TYPE_PREVIEW))

typedef struct _BlPreviewClass  BlPreviewClass;

struct _BlPreviewClass
{
  GtkDrawingAreaClass parent_class;
};

struct _BlPreview
{
  GtkDrawingArea      parent_instance;

  GdkPixmap          *back_buffer;

  gint                width;
  gint                height;
  guchar             *data;
};


GtkType     bl_preview_get_type   (void);
GtkWidget * bl_preview_new        (void);
void        bl_preview_set_data   (BlPreview *preview,
                                   gint       width,
                                   gint       height,
                                   guchar    *data);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BL_PREVIEW_H__ */

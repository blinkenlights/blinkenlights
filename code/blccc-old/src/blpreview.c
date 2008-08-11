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

#include <gtk/gtk.h>

#include "bltypes.h"
#include "blpreview.h"


static void     bl_preview_class_init    (BlPreviewClass *class);
static void     bl_preview_init          (BlPreview      *preview);
static void     bl_preview_destroy       (GtkObject      *object);
static gboolean bl_preview_expose_event  (GtkWidget      *widget,
                                          GdkEventExpose *event);
static void     bl_preview_size_allocate (GtkWidget      *widget,
                                          GtkAllocation  *allocation);


static GtkDrawingAreaClass *parent_class = NULL;


GtkType
bl_preview_get_type (void)
{
  static GtkType preview_type = 0;

  if (!preview_type)
    {
      GtkTypeInfo preview_info =
      {
	"BlPreview",
	sizeof (BlPreview),
	sizeof (BlPreviewClass),
	(GtkClassInitFunc) bl_preview_class_init,
	(GtkObjectInitFunc) bl_preview_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL
      };

      preview_type = gtk_type_unique (gtk_drawing_area_get_type (), 
                                      &preview_info);
    }
  
  return preview_type;
}

static void
bl_preview_class_init (BlPreviewClass *class)
{
  GtkObjectClass *object_class;
  GtkWidgetClass *widget_class;

  parent_class = gtk_type_class (gtk_drawing_area_get_type ());

  object_class = GTK_OBJECT_CLASS (class);
  widget_class = GTK_WIDGET_CLASS (class);

  object_class->destroy = bl_preview_destroy;

  widget_class->size_allocate = bl_preview_size_allocate;
  widget_class->expose_event  = bl_preview_expose_event;
}

static void
bl_preview_init (BlPreview *preview)
{
  preview->back_buffer = NULL;

  preview->width       = 0;
  preview->height      = 0;
  preview->data        = 0;
}

static void
bl_preview_destroy (GtkObject *object)
{
  BlPreview *preview;
  
  preview = BL_PREVIEW (object);

  if (preview->back_buffer)
    gdk_pixmap_unref (preview->back_buffer);

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
bl_preview_size_allocate (GtkWidget     *widget,
                          GtkAllocation *allocation)
{
  BlPreview *preview;
  gint       width  = 0;
  gint       height = 0;
  
  if (!GTK_WIDGET_DRAWABLE (widget))
    return;

  preview = BL_PREVIEW (widget);
  
  if (preview->back_buffer)
    gdk_window_get_size (preview->back_buffer, &width, &height);

  if (width != allocation->width || height != allocation->height)
    {
      if (preview->back_buffer)
        gdk_pixmap_unref (preview->back_buffer);

      preview->back_buffer = gdk_pixmap_new (widget->window,
                                             allocation->width, 
                                             allocation->height, -1);
    }

  if (GTK_WIDGET_CLASS (parent_class)->size_allocate)
    GTK_WIDGET_CLASS (parent_class)->size_allocate (widget, allocation);
}

static gboolean
bl_preview_expose_event (GtkWidget      *widget,
                         GdkEventExpose *event)
{
  BlPreview    *preview;
  GdkRectangle  rect;
  const gchar  *data;
  gint          width;
  gint          height;
  gint          x, y;

  if (!GTK_WIDGET_DRAWABLE (widget))
    return FALSE;

  preview = BL_PREVIEW (widget);

  if (!preview->back_buffer)
    return FALSE;

  gdk_draw_rectangle (preview->back_buffer,
                      widget->style->black_gc,
                      TRUE,
                      event->area.x, event->area.y,
                      event->area.width, event->area.height);

  if (preview->data && preview->width && preview->height)
    {
      data = preview->data;
      
      width  = widget->allocation.width  / preview->width;
      height = widget->allocation.height / preview->height;
      
      for (y = 0; y < preview->height; y++)
        for (x = 0; x < preview->width; x++)
          {
            rect.x = x * width  + width  / 16;
            rect.y = y * height + height / 16;
            rect.width  = width  * 7 / 8;
            rect.height = height * 7 / 8;
            
            if (gdk_rectangle_intersect (&event->area, &rect, &rect))
              gdk_draw_rectangle (preview->back_buffer,
                                  (*data ? 
                                   widget->style->white_gc : 
                                   widget->style->black_gc),
                                  TRUE,
                                  rect.x, rect.y, rect.width, rect.height);
            
            data++;
          }
    }
  
  gdk_draw_pixmap (widget->window,
                   widget->style->bg_gc[GTK_STATE_NORMAL],
                   preview->back_buffer,
                   event->area.x, event->area.y,
                   event->area.x, event->area.y,
                   event->area.width, event->area.height);
  
  return TRUE;
}

GtkWidget *
bl_preview_new (void)
{
  return GTK_WIDGET (gtk_object_new (BL_TYPE_PREVIEW, NULL));
}

void
bl_preview_set_data (BlPreview *preview,
                     gint       width,
                     gint       height,
                     guchar    *data) 
{
  g_return_if_fail (preview != NULL);
  g_return_if_fail (BL_IS_PREVIEW (preview));

  preview->width  = data ? width  : 0;
  preview->height = data ? height : 0;
  preview->data   = data;

  gtk_widget_queue_draw (GTK_WIDGET (preview));
}

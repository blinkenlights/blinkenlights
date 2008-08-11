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

#include "config.h"

#include <string.h>

#include <gtk/gtk.h>

#include "blib/blib.h"

#include "bview-pixbuf.h"
#include "bview-gtk.h"


static void          b_view_gtk_class_init   (BViewGtkClass  *class);
static void          b_view_gtk_init         (BViewGtk       *view);
static void          b_view_gtk_finalize     (GObject        *object);
static void          b_view_gtk_realize      (GtkWidget      *widget);
static void          b_view_gtk_size_request (GtkWidget      *widget,
                                              GtkRequisition *requisition);
static gboolean      b_view_gtk_expose_event (GtkWidget      *widget,
                                              GdkEventExpose *event);
static gboolean      b_view_gtk_alloc_color  (GdkDrawable    *drawable,
                                              BColor         *color,
                                              GdkColor       *gdk_color);
static GdkDrawable * b_view_gtk_get_drawable (BViewGtk       *view,
                                              const gchar    *filename,
                                              GdkWindow      *window);


static GtkDrawingAreaClass *parent_class = NULL;


GType
b_view_gtk_get_type (void)
{
  static GType view_type = 0;

  if (!view_type)
    {
      static const GTypeInfo view_info =
      {
        sizeof (BViewGtkClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_view_gtk_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BViewGtk),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_view_gtk_init,
      };

      view_type = g_type_register_static (GTK_TYPE_DRAWING_AREA,
                                          "BViewGtk", &view_info, 0);
    }

  return view_type;
}

static void
b_view_gtk_class_init (BViewGtkClass *class)
{
  GObjectClass   *object_class;
  GtkWidgetClass *widget_class;

  parent_class = g_type_class_peek_parent (class);
  object_class = G_OBJECT_CLASS (class);
  widget_class = GTK_WIDGET_CLASS (class);

  object_class->finalize = b_view_gtk_finalize;

  widget_class->realize      = b_view_gtk_realize;
  widget_class->size_request = b_view_gtk_size_request;
  widget_class->expose_event = b_view_gtk_expose_event;
}

static void
b_view_gtk_init (BViewGtk *view)
{
  view->pixview    = NULL;
  view->theme      = NULL;
  view->frame_data = NULL;
  view->drawables  = g_hash_table_new_full (g_str_hash, g_str_equal,
                                            (GDestroyNotify) NULL,
                                            (GDestroyNotify) g_object_unref);
}

static void
b_view_gtk_finalize (GObject *object)
{
  BViewGtk *view = B_VIEW_GTK (object);

  g_free (view->frame_data);

  g_hash_table_destroy (view->drawables);

  g_object_unref (view->theme);
  g_object_unref (view->pixview);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
b_view_gtk_realize (GtkWidget *widget)
{
  BViewGtk *view  = B_VIEW_GTK (widget);
  GdkColor  color;

  GTK_WIDGET_CLASS (parent_class)->realize (widget);

  if (b_view_gtk_alloc_color (GDK_DRAWABLE (widget->window),
                              &view->theme->bg_color, &color))
    {
      gdk_window_set_background (widget->window, &color);
    }
}

static void
b_view_gtk_size_request (GtkWidget      *widget,
                         GtkRequisition *requisition)
{
  BViewGtk *view = B_VIEW_GTK (widget);

  requisition->width  = view->theme->width;
  requisition->height = view->theme->height;
}

static gboolean
b_view_gtk_expose_event (GtkWidget      *widget,
                         GdkEventExpose *event)
{
  BViewGtk   *view  = B_VIEW_GTK (widget);
  BTheme     *theme = view->theme;
  GdkGC      *gc;
  BRectangle  rect;
  gint        x, y;

  x = (widget->allocation.width  - view->theme->width)  / 2;
  y = (widget->allocation.height - view->theme->height) / 2;

  gc = widget->style->fg_gc[GTK_STATE_NORMAL];

  gdk_gc_set_clip_rectangle (gc, &event->area);

  if (view->blend)
    {
      rect.x = x;
      rect.y = y;
      rect.w = view->theme->width;
      rect.h = view->theme->height;

      if (b_rectangle_intersect (&rect, (BRectangle *) &event->area, NULL))
        gdk_draw_pixbuf (widget->window, gc,
                         view->pixview->pixbuf,
                         0, 0, rect.x, rect.y, rect.w, rect.h,
                         GDK_RGB_DITHER_NORMAL,
                         - event->area.x, - event->area.y);
    }
  else
    {
      GdkDrawable *drawable;
      GList       *list;

      drawable = b_view_gtk_get_drawable (view,
                                          theme->bg_image, widget->window);

      if (drawable)
        {
          rect.x = x + view->theme->bg_image_x;
          rect.y = y + view->theme->bg_image_y;

          gdk_drawable_get_size (drawable, &rect.w, &rect.h);

          if (b_rectangle_intersect (&rect, (BRectangle *) &event->area, NULL))
            gdk_draw_drawable (GDK_DRAWABLE (widget->window), gc,
                               drawable,
                               0, 0,
                               rect.x, rect.y, rect.w, rect.h);
        }
      for (list = theme->overlays; list; list = list->next)
        {
          BOverlay *overlay = list->data;
          GList    *windows;
          GdkColor  color;

          drawable = b_view_gtk_get_drawable (view,
                                              overlay->image, widget->window);

          if (drawable == NULL &&
              b_view_gtk_alloc_color (widget->window, &overlay->color, &color))
            {
              gdk_gc_set_foreground (gc, &color);
            }

          for (windows = overlay->windows; windows; windows = windows->next)
            {
              BWindow *window = windows->data;
              guchar   value;

              value = view->frame_data[(window->column +
                                        window->row * theme->columns)];
              if (!value)
                continue;

              window += (value * theme->maxval) / 256;

              rect.x = window->rect.x + x;
              rect.y = window->rect.y + y;
              rect.w = window->rect.w;
              rect.h = window->rect.h;

              if (b_rectangle_intersect (&rect,
                                         (BRectangle *) &event->area, NULL))
                {
                  if (drawable)
                    gdk_draw_drawable (GDK_DRAWABLE (widget->window), gc,
                                       drawable,
                                       window->src_x, window->src_y,
                                       rect.x, rect.y, rect.w, rect.h);
                  else
                    gdk_draw_rectangle (GDK_DRAWABLE (widget->window), gc,
                                        TRUE,
                                        rect.x, rect.y, rect.w, rect.h);
                }
            }
        }
    }

  gdk_gc_set_clip_rectangle (gc, NULL);

  return TRUE;
}

/**
 * b_view_gtk_new:
 * @theme: a #BTheme object
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Creates a new widget suitable to display Blinkenlights movies that
 * fit the @theme.
 *
 * Return value: a new widget or %NULL in case of an error
 **/
GtkWidget *
b_view_gtk_new (BTheme  *theme,
                GError **error)
{
  BViewGtk    *view;
  BViewPixbuf *pixview;

  g_return_val_if_fail (B_IS_THEME (theme), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (theme->channels != 1)
    {
      g_set_error (error, 0, 0, "Channels != 1 is not (yet) supported");
      return NULL;
    }

  pixview = b_view_pixbuf_new_theme (theme, TRUE, error);
  if (! pixview)
    return FALSE;

  view = B_VIEW_GTK (g_object_new (B_TYPE_VIEW_GTK, NULL));

  view->pixview    = pixview;
  view->theme      = g_object_ref (theme);
  view->blend      = theme->needs_blending;
  view->frame_data = g_new0 (guchar, (theme->rows * theme->columns));

  if (view->blend)
    b_view_pixbuf_render (view->pixview, NULL, NULL, NULL);

  return GTK_WIDGET (view);
}

/**
 * b_view_gtk_update:
 * @view: a #BViewGtk widget
 * @frame_data: the frame data to display
 *
 * Displays a new frame on the @view. The @view expects @frame_data
 * in the range of 0 to 255.
 **/
void
b_view_gtk_update (BViewGtk     *view,
                   const guchar *frame_data)
{
  GtkWidget  *widget;
  BTheme     *theme;
  BRectangle  rect;

  g_return_if_fail (B_IS_VIEW_GTK (view));
  g_return_if_fail (B_IS_THEME (view->theme));

  theme = view->theme;

  b_theme_frame_diff_boundary (theme, view->frame_data, frame_data, &rect);

  if (frame_data)
    memcpy (view->frame_data, frame_data, (theme->rows * theme->columns));
  else
    memset (view->frame_data, 0, (theme->rows * theme->columns));

  if (view->blend)
    b_view_pixbuf_render (view->pixview, view->frame_data, &rect, NULL);

  widget = GTK_WIDGET (view);

  rect.x += (widget->allocation.width  - view->theme->width)  / 2;
  rect.y += (widget->allocation.height - view->theme->height) / 2;

  gtk_widget_queue_draw_area (widget, rect.x, rect.y, rect.w, rect.h);
}

static gboolean
b_view_gtk_alloc_color (GdkDrawable *drawable,
                        BColor      *color,
                        GdkColor    *gdk_color)
{
  gdk_color->red   = (color->r << 8) | color->r;
  gdk_color->green = (color->g << 8) | color->g;
  gdk_color->blue  = (color->b << 8) | color->b;

  return (gdk_colormap_alloc_color (gdk_drawable_get_colormap (drawable),
                                    gdk_color, FALSE, TRUE));
}

static GdkDrawable *
b_view_gtk_get_drawable (BViewGtk    *view,
                         const gchar *filename,
                         GdkWindow   *window)
{
  GdkDrawable     *drawable;
  const GdkPixbuf *pixbuf;

  if (! filename)
    return NULL;

  drawable = g_hash_table_lookup (view->drawables, filename);
  if (drawable)
    return drawable;

  pixbuf = b_view_pixbuf_load_image (view->pixview, filename, NULL);
  if (pixbuf)
    {
      GdkColormap *colormap = gdk_drawable_get_colormap (window);
      gint         width    = gdk_pixbuf_get_width (pixbuf);
      gint         height   = gdk_pixbuf_get_height (pixbuf);
      GdkGC       *gc;

      drawable = GDK_DRAWABLE (gdk_pixmap_new (window, width, height,
                                               gdk_colormap_get_visual (colormap)->depth));

      gdk_drawable_set_colormap (drawable, colormap);

      gc = gdk_gc_new (drawable);

      gdk_draw_pixbuf (drawable, gc,
                       (GdkPixbuf *) pixbuf, 0, 0, 0, 0, width, height,
                       GDK_RGB_DITHER_NORMAL, 0, 0);

      g_object_unref (gc);

      g_hash_table_insert (view->drawables, (gpointer) filename, drawable);
    }

  return drawable;
}

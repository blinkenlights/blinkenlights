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

#include <gdk-pixbuf/gdk-pixbuf.h>

#include "blib/blib.h"

#include "bview-pixbuf.h"


static void   b_view_pixbuf_class_init (BViewPixbufClass  *class);
static void   b_view_pixbuf_init       (BViewPixbuf       *view);
static void   b_view_pixbuf_finalize   (GObject           *object);

static const GdkPixbuf *
              b_view_pixbuf_load_background (BViewPixbuf       *view,
                                             const gchar       *filename,
                                             const BColor      *color,
                                             GError           **error);
static void   b_view_pixbuf_fill_rect       (GdkPixbuf         *pixbuf,
                                             const BRectangle  *rect,
                                             const BColor      *color);
static void   b_view_pixbuf_blend_rect      (GdkPixbuf         *dest,
                                             const GdkPixbuf   *src,
                                             const BRectangle  *dest_rect,
                                             gint               src_x,
                                             gint               src_y,
                                             guchar             opacity);


static GObjectClass *parent_class = NULL;


GType
b_view_pixbuf_get_type (void)
{
  static GType view_type = 0;

  if (!view_type)
    {
      static const GTypeInfo view_info =
      {
        sizeof (BViewPixbufClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_view_pixbuf_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BViewPixbuf),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_view_pixbuf_init,
      };

      view_type = g_type_register_static (G_TYPE_OBJECT,
                                          "BViewPixbuf", &view_info, 0);
    }

  return view_type;
}

static void
b_view_pixbuf_class_init (BViewPixbufClass *class)
{
  GObjectClass *object_class = G_OBJECT_CLASS (class);

  parent_class = g_type_class_peek_parent (class);

  object_class->finalize = b_view_pixbuf_finalize;
}

static void
b_view_pixbuf_init (BViewPixbuf *view)
{
  view->pixbuf   = NULL;
  view->theme    = NULL;
  view->rows     = 0;
  view->columns  = 0;
  view->channels = 0;
  view->images   = g_hash_table_new_full (g_str_hash, g_str_equal,
                                          (GDestroyNotify) NULL,
                                          (GDestroyNotify) g_object_unref);
}

static void
b_view_pixbuf_finalize (GObject *object)
{
  BViewPixbuf *view = B_VIEW_PIXBUF (object);

  g_hash_table_destroy (view->images);

  if (view->theme)
    g_object_unref (view->theme);

  if (view->pixbuf)
    g_object_unref (view->pixbuf);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * b_view_pixbuf_new:
 * @rows:
 * @columns:
 * @channels:
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Creates a new #BViewPixbuf for the given parameters. This is a
 * simple version that renders a grayscale pixel for every window.
 *
 * Return value: a new view or %NULL in case of an error
 **/
BViewPixbuf *
b_view_pixbuf_new (gint     rows,
                   gint     columns,
                   gint     channels,
                   GError **error)
{
  BViewPixbuf *view;

  g_return_val_if_fail (rows > 0 && columns > 0, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (channels != 1)
    {
      g_set_error (error, 0, 0, "Channels != 1 is not (yet) supported");
      return NULL;
    }

  view = B_VIEW_PIXBUF (g_object_new (B_TYPE_VIEW_PIXBUF, NULL));

  view->rows     = rows;
  view->columns  = columns;
  view->channels = channels;

  return view;
}

/**
 * b_view_pixbuf_new_theme:
 * @theme:         a #BTheme object
 * @image_preload: whether to preload all images
 * @error:         location to store the error occuring,
 *                 or %NULL to ignore errors
 *
 * Creates a pixbuf renderer for Blinkenlights frames using the
 * @theme.  When image_preload is enabled, all images refered to by
 * the theme are loaded. If an error occurs loading the images,
 * b_view_pixbuf_new() will fail and return %NULL. If a view is
 * created with preloaded images, calling b_view_pixbuf_render() on
 * it will always succeed so you can skip error checking later.
 *
 * Return value: a new view or %NULL in case of an error
 **/
BViewPixbuf *
b_view_pixbuf_new_theme (BTheme    *theme,
                         gboolean   image_preload,
                         GError   **error)
{
  BViewPixbuf *view;

  g_return_val_if_fail (B_IS_THEME (theme), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (theme->channels != 1)
    {
      g_set_error (error, 0, 0, "Channels != 1 is not (yet) supported");
      return NULL;
    }

  view = B_VIEW_PIXBUF (g_object_new (B_TYPE_VIEW_PIXBUF, NULL));

  view->theme = g_object_ref (theme);

  view->rows     = theme->rows;
  view->columns  = theme->columns;
  view->channels = theme->channels;

  if (image_preload)
    {
      GList *list;

      if (theme->bg_image)
        {
          if (! b_view_pixbuf_load_background (view,
                                               theme->bg_image,
                                               &theme->bg_color,
                                               error))
            {
              g_object_unref (view);
              return NULL;
            }
        }

      for (list = theme->overlays; list; list = list->next)
        {
          BOverlay *overlay = list->data;

          if (! b_view_pixbuf_load_image (view, overlay->image, error))
            {
              g_object_unref (view);
              return NULL;
            }
        }
    }

  return view;
}

static const GdkPixbuf *
b_view_pixbuf_render_theme (BViewPixbuf       *view,
                            const guchar      *frame_data,
                            const BRectangle  *clip,
                            GError           **error)
{
  BTheme          *theme = view->theme;
  const GdkPixbuf *image;
  GList           *list;
  BRectangle       rect;

  if (view->pixbuf)
    {
      b_view_pixbuf_fill_rect (view->pixbuf, clip, &theme->bg_color);
    }
  else
    {
      view->pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8,
                                     theme->width, theme->height);
      b_view_pixbuf_fill_rect (view->pixbuf, NULL, &theme->bg_color);
    }

  if (theme->bg_image)
    {
      gint x, y;

      image = b_view_pixbuf_load_background (view,
                                             theme->bg_image, &theme->bg_color,
                                             error);
      if (! image)
        return NULL;

      rect.x = x = theme->bg_image_x;
      rect.y = y = theme->bg_image_y;
      rect.w = gdk_pixbuf_get_width (image);
      rect.h = gdk_pixbuf_get_height (image);

      if (!clip || b_rectangle_intersect (&rect, clip, &rect))
        gdk_pixbuf_copy_area (image,
                              rect.x - x, rect.y - x, rect.w, rect.h,
                              view->pixbuf,
                              rect.x, rect.y);
    }

  for (list = theme->overlays; list && frame_data; list = list->next)
    {
      BOverlay  *overlay = list->data;
      GList     *windows;

      if (overlay->image)
        {
          image = b_view_pixbuf_load_image (view, overlay->image, error);
          if (! image)
            return NULL;

          for (windows = overlay->windows; windows; windows = windows->next)
            {
              BWindow *window = windows->data;
              guchar   value  = frame_data[(window->column +
                                            window->row * theme->columns)];
              if (value)
                {
                  gint x, y;

                  window += (value * theme->maxval) / 256;

                  memcpy (&rect, &window->rect, sizeof (BRectangle));

                  x = rect.x;
                  y = rect.y;

                  if (!clip || b_rectangle_intersect (&rect, clip, &rect))
                    b_view_pixbuf_blend_rect (view->pixbuf, image,
                                              &rect,
                                              window->src_x + rect.x - x,
                                              window->src_y + rect.y - y,
                                              (window->value
                                               == B_WINDOW_VALUE_ALL) ?
                                              value : 0xFF);
                }
            }
        }
      else
        {
          BColor color = overlay->color;

          for (windows = overlay->windows; windows; windows = windows->next)
            {
              BWindow *window = windows->data;
              guchar   value  = frame_data[(window->column +
                                            window->row * theme->columns)];
              if (value)
                {
                  window += (value * theme->maxval) / 256;

                  color.a = overlay->color.a;
                  if (window->value == B_WINDOW_VALUE_ALL)
                    color.a = ((color.a + 1) * value) >> 8;

                  memcpy (&rect, &window->rect, sizeof (BRectangle));

                  if (!clip || b_rectangle_intersect (&rect, clip, &rect))
                    b_view_pixbuf_fill_rect (view->pixbuf, &rect, &color);
                }
            }
        }

    }

  return (const GdkPixbuf *) view->pixbuf;
}


/**
 * b_view_pixbuf_render
 * @view:       a #BViewPixbuf object
 * @frame_data: the frame data to display
 * @clip:       clipping rectangle (or %NULL)
 * @error:      location to store the error occuring, or %NULL to ignore
 *              errors
 *
 * Renders a frame on the @view. The @view expects @frame_data in the
 * range of 0 to 255.
 *
 * Return value: a reference to a #GdkPixbuf with the rendered frame.
 **/
const GdkPixbuf *
b_view_pixbuf_render (BViewPixbuf       *view,
                      const guchar      *frame_data,
                      const BRectangle  *clip,
                      GError           **error)
{
  BRectangle  rect;

  g_return_val_if_fail (B_IS_VIEW_PIXBUF (view), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (view->theme)
    return b_view_pixbuf_render_theme (view, frame_data, clip, error);

  if (! view->pixbuf)
    view->pixbuf = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8,
                                   view->columns, view->rows);

  rect.x = 0;
  rect.y = 0;
  rect.w = view->columns;
  rect.h = view->rows;

  if (!clip || b_rectangle_intersect (&rect, clip, &rect))
    {
      const guchar *src;
      guchar       *dest;
      guchar       *pixels = gdk_pixbuf_get_pixels (view->pixbuf);
      gint          pitch  = gdk_pixbuf_get_rowstride (view->pixbuf);
      gint          x, y;

      src  = frame_data + (rect.y * view->columns) + rect.x;
      dest = pixels + (rect.y * pitch) + rect.x;

      for (y = 0; y < rect.h; y++)
        {
          for (x = 0; x < rect.w; x++)
            {
              dest[3 * x]     = src[x];
              dest[3 * x + 1] = src[x];
              dest[3 * x + 2] = src[x];
            }

          dest += pitch;
          src += view->columns;
        }
    }

  return (const GdkPixbuf *) view->pixbuf;
}

/**
 * b_view_pixbuf_load_image:
 * @view:     a #BViewPixbuf object
 * @filename: the filename of the image to load
 * @error:    location to store the error occuring, or %NULL to ignore errors
 *
 * Renders a frame on the @view and returns a pointer to a #GdkPixbuf
 * holding the rendered image. The pixbuf is owned by the view, you
 * may only read from it.
 *
 * The @view expects @frame_data in the range of 0 to 255.
 *
 * Return value: a pointer to a #GdkPixbuf with the rendered frame
 **/
const GdkPixbuf *
b_view_pixbuf_load_image (BViewPixbuf  *view,
                          const gchar  *filename,
                          GError      **error)
{
  GdkPixbuf *pixbuf = g_hash_table_lookup (view->images, filename);

  if (pixbuf)
    return pixbuf;

  pixbuf = gdk_pixbuf_new_from_file (filename, error);

  if (pixbuf)
    {
      g_hash_table_insert (view->images, (gpointer) filename, pixbuf);
    }
  else if (error && *error)
    {
      gchar *msg = g_strdup_printf ("Error loading image from file '%s': %s\n",
                                    filename, (*error)->message);
      g_free ((*error)->message);
      (*error)->message = msg;
    }

  return (const GdkPixbuf *) pixbuf;
}

static const GdkPixbuf *
b_view_pixbuf_load_background (BViewPixbuf   *view,
                               const gchar   *filename,
                               const BColor  *color,
                               GError       **error)
{
  const GdkPixbuf *pixbuf;
  GdkPixbuf       *flat;
  BRectangle       rect;

  pixbuf = b_view_pixbuf_load_image (view, filename, error);

  if (! pixbuf)
    return NULL;

  if (! gdk_pixbuf_get_has_alpha (pixbuf))
    return pixbuf;

  rect.x = 0;
  rect.y = 0;
  rect.w = gdk_pixbuf_get_width  (pixbuf);
  rect.h = gdk_pixbuf_get_height (pixbuf);

  flat = gdk_pixbuf_new (GDK_COLORSPACE_RGB, FALSE, 8, rect.w, rect.h);

  b_view_pixbuf_fill_rect (flat, &rect, color);
  b_view_pixbuf_blend_rect (flat, pixbuf, &rect, 0, 0, 0xFF);

  g_hash_table_insert (view->images, (gpointer) filename, flat);

  return (const GdkPixbuf *) flat;
}


/* this function assumes that the pixbuf has no alpha channel */
static void
b_view_pixbuf_fill_rect (GdkPixbuf        *pixbuf,
                         const BRectangle *rect,
                         const BColor     *color)
{
  guchar     *data;
  gint        pitch;
  guchar      r, g, b;
  BRectangle  fill  = { 0, 0,
                        gdk_pixbuf_get_width  (pixbuf),
                        gdk_pixbuf_get_height (pixbuf) };

  if (rect && ! b_rectangle_intersect ((BRectangle *) rect, &fill, &fill))
    return;

  data  = gdk_pixbuf_get_pixels (pixbuf);
  pitch = gdk_pixbuf_get_rowstride (pixbuf);

  data += fill.y * pitch + fill.x * 3;

  r = color ? color->r : 0x0;
  g = color ? color->g : 0x0;
  b = color ? color->b : 0x0;

  do
    {
      guchar *d = data;
      guint   w = fill.w;

      do
        {
          data[0] = r;
          data[1] = g;
          data[2] = g;

          d += 3;
        }
      while (--w);

      data += pitch;
    }
  while (--fill.h);
}

/* this function assumes that the dest pixbuf has no alpha channel */
static void
b_view_pixbuf_blend_rect (GdkPixbuf        *dest,
                          const GdkPixbuf  *src,
                          const BRectangle *dest_rect,
                          gint              src_x,
                          gint              src_y,
                          guchar            opacity)
{
  guchar     *src_data;
  guchar     *dest_data;
  gint        src_pitch;
  gint        dest_pitch;
  gint        i;
  BRectangle  rect = { 0, 0,
                       gdk_pixbuf_get_width  (dest),
                       gdk_pixbuf_get_height (dest) };

  if (! opacity)
    return;

  if (! b_rectangle_intersect (dest_rect, &rect, &rect))
    return;

  src_x += rect.x - dest_rect->x;
  src_y += rect.y - dest_rect->y;

  if (src_x < 0)
    rect.w += src_x, src_x = 0;

  if (src_y < 0)
    rect.h += src_y, src_y = 0;

  i = gdk_pixbuf_get_width (src);
  rect.w = MIN (i - src_x, rect.w);
  if (rect.w < 1)
    return;

  i = gdk_pixbuf_get_height (src);
  rect.h = MIN (i - src_y, rect.h);
  if (rect.h < 1)
    return;

  src_data   = gdk_pixbuf_get_pixels (src);
  src_pitch  = gdk_pixbuf_get_rowstride (src);

  dest_data  = gdk_pixbuf_get_pixels (dest);
  dest_pitch = gdk_pixbuf_get_rowstride (dest);

  dest_data += rect.y * dest_pitch + rect.x * 3;

  switch (gdk_pixbuf_get_n_channels (src))
    {
    case 3:
      src_data += src_y * src_pitch + src_x * 3;

      switch (opacity)
        {
        case 0xff:
          do
            {
              guchar *s = src_data;
              guchar *d = dest_data;
              gint    w = rect.w;

              do
                {
                  d[0] = s[0];
                  d[1] = s[1];
                  d[2] = s[2];

                  d += 3;
                  s += 3;
                }
              while (--w);

              src_data  += src_pitch;
              dest_data += dest_pitch;
            }
          while (--rect.h);
          break;

        default:
          do
            {
              guchar *s = src_data;
              guchar *d = dest_data;
              gint    w = rect.w;

              do
                {
                  guint a0 = opacity;
                  guint a1 = 0xff - opacity;
                  guint a  = a0 + a1;

		  d[0] = (a0 * s[0] + a1 * d[0]) / a;
		  d[1] = (a0 * s[1] + a1 * d[1]) / a;
		  d[2] = (a0 * s[2] + a1 * d[2]) / a;

                  d += 3;
                  s += 3;
                }
              while (--w);

              src_data  += src_pitch;
              dest_data += dest_pitch;
            }
          while (--rect.h);
          break;
        }
      break;

    case 4:
      src_data += src_y * src_pitch + src_x * 4;

      switch (opacity)
        {
        case 0xff:
          do
            {
              guchar *s = src_data;
              guchar *d = dest_data;
              gint    w = rect.w;

              do
                {
                  switch (s[3])
                    {
                    case 0:
                      break;

                    case 0xff:
                      d[0] = s[0];
                      d[1] = s[1];
                      d[2] = s[2];
                      break;

                    default:
                      {
                        guint a0 = s[3] * 0xff;
                        guint a1 = 0xffff - s[3];
                        guint a  = a0 + a1;

                        d[0] = (a0 * s[0] + a1 * d[0]) / a;
                        d[1] = (a0 * s[1] + a1 * d[1]) / a;
                        d[2] = (a0 * s[2] + a1 * d[2]) / a;
                      }
                      break;
                    }

                  s += 4;
                  d += 3;
                }
              while (--w);

              src_data  += src_pitch;
              dest_data += dest_pitch;
            }
          while (--rect.h);
          break;

        default:
          do
            {
              guchar *s = src_data;
              guchar *d = dest_data;
              gint    w = rect.w;

              do
                {
                  if (s[3])
                    {
                      guint a0 = s[3] * opacity;
                      guint a1 = 0xffff - opacity;
                      guint a  = a0 + a1;

                      d[0] = (a0 * s[0] + a1 * d[0]) / a;
                      d[1] = (a0 * s[1] + a1 * d[1]) / a;
                      d[2] = (a0 * s[2] + a1 * d[2]) / a;
                    }

                  s += 4;
                  d += 3;
                }
              while (--w);

              src_data  += src_pitch;
              dest_data += dest_pitch;
            }
          while (--rect.h);
          break;
        }
    }
}

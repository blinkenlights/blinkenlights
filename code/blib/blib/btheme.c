/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (C) 2002-2004  The Blinkenlights Crew
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

#include <glib-object.h>

#include "btypes.h"
#include "bobject.h"
#include "btheme.h"
#include "btheme-parser.h"
#include "butils.h"


static void   b_theme_class_init (BThemeClass *class);
static void   b_theme_init       (BTheme      *theme);
static void   b_theme_finalize   (GObject     *object);


static BObjectClass *parent_class = NULL;

GType
b_theme_get_type (void)
{
  static GType theme_type = 0;

  if (!theme_type)
    {
      static const GTypeInfo theme_info =
      {
        sizeof (BThemeClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_theme_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BTheme),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_theme_init
      };

      theme_type = g_type_register_static (B_TYPE_OBJECT,
                                           "BTheme", &theme_info, 0);
    }

  return theme_type;
}

static void
b_theme_class_init (BThemeClass *class)
{
  GObjectClass *object_class;

  parent_class = g_type_class_peek_parent (class);
  object_class = G_OBJECT_CLASS (class);

  object_class->finalize = b_theme_finalize;
}


static void
b_theme_init (BTheme *theme)
{
  theme->bg_color.a     = 0xFF;
  theme->bg_image_x     = 0;
  theme->bg_image_y     = 0;
  theme->needs_blending = FALSE;
}

static void
b_theme_finalize (GObject *object)
{
  BTheme *theme;

  theme = B_THEME (object);

  if (theme->type)
    {
      g_free (theme->type);
      theme->type = NULL;
    }

  b_theme_unload (theme);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
b_theme_needs_blending (BTheme *theme)
{
  GList *list;

  if (theme->maxval == 1)
    return FALSE;

  for (list = theme->overlays; list; list = list->next)
    {
      BOverlay *overlay = list->data;
      GList    *windows;

      for (windows = overlay->windows; windows; windows = windows->next)
        {
          BWindow *window = windows->data;

          if (window->value == B_WINDOW_VALUE_ALL)
            return TRUE;
        }
    }

  return FALSE;
}


/**
 * b_theme_new_from_file:
 * @filename: the name of the file to load the theme from
 * @lazy_load: whether to do lazy-loading
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Tries to load a #BTheme from the file pointed to by @filename. If
 * @lazy_load is %TRUE, only the header is loaded.
 *
 * Return value: a newly allocated #BTheme object or %NULL if the load
 * failed
 **/
BTheme *
b_theme_new_from_file (const gchar  *filename,
                       gboolean      lazy_load,
                       GError      **error)
{
  BTheme *theme;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (g_path_is_absolute (filename))
    {
      theme = g_object_new (B_TYPE_THEME, "filename", filename, NULL);
    }
  else
    {
      gchar *cwd = g_get_current_dir ();
      gchar *abs = g_build_filename (cwd, filename, NULL);

      theme = g_object_new (B_TYPE_THEME, "filename", abs, NULL);

      g_free (abs);
      g_free (cwd);
    }

  if (! b_theme_parser_parse (theme, lazy_load, error))
    {
      g_object_unref (theme);
      return NULL;
    }

  theme->needs_blending = b_theme_needs_blending (theme);

  return theme;
}

/**
 * b_theme_new_from_scratch:
 * @title: a descriptive title
 * @type: the theme type or %NULL
 * @rows: the number of rows of windows
 * @columns: the number of columns of windows
 * @channels: the number of channels per window (must be 1)
 * @maxval: the maximum value
 * @width: screen width in pixels
 * @height: screen height in pixels
 *
 * Creates a new #BTheme object from scratch. This may be useful if
 * you want to quickly test a movie for a layout you don't have a
 * theme for. You need to call b_theme_load() before you can use the
 * new theme.
 *
 * Return value: a newly allocated, lazy-loaded, #BTheme object
 **/
BTheme *
b_theme_new_from_scratch (const gchar *title,
                          const gchar *type,
                          gint         rows,
                          gint         columns,
                          gint         channels,
                          gint         maxval,
                          gint         width,
                          gint         height)
{
  BTheme   *theme;

  g_return_val_if_fail (title != NULL, NULL);
  g_return_val_if_fail (rows > 0 && columns > 0, NULL);
  g_return_val_if_fail (channels == 1, NULL);
  g_return_val_if_fail (maxval > 0 && maxval < 256, NULL);
  g_return_val_if_fail (width > 0 && height > 0, NULL);

  theme = g_object_new (B_TYPE_THEME, "name", title, NULL);

  theme->type     = g_strdup (type);
  theme->rows     = rows;
  theme->columns  = columns;
  theme->channels = channels;
  theme->maxval   = maxval;
  theme->width    = width;
  theme->height   = height;

  theme->needs_blending = b_theme_needs_blending (theme);

  return theme;
}

static void
b_theme_load_from_scratch (BTheme *theme)
{
  BOverlay *overlay;
  gint      row;
  gint      column;
  gint      i;

  overlay = g_new0 (BOverlay, 1);

  overlay->color.a = 0xFF;
  overlay->color.r = 0xFF;
  overlay->color.g = 0xFF;
  overlay->color.b = 0xFF;

  for (row = 0; row < theme->rows; row++)
    for (column = 0; column < theme->columns; column++)
      {
        BWindow *windows;
        BWindow  template;

        template.value  = B_WINDOW_VALUE_ALL;
        template.row    = row;
        template.column = column;
        template.src_x  = template.src_y = 0;
        template.rect.w = theme->width  / theme->columns;
        template.rect.h = theme->height / theme->rows;
        template.rect.x = column * template.rect.w;
        template.rect.y = row    * template.rect.h;

        windows = g_new0 (BWindow, theme->maxval);

        for (i = 0; i < theme->maxval; i++)
          windows[i] = template;

        overlay->windows = g_list_prepend (overlay->windows, windows);
      }

  overlay->windows = g_list_reverse (overlay->windows);

  theme->overlays = g_list_prepend (NULL, overlay);
}

/**
 * b_theme_load:
 * @theme: a #BTheme object
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Loads all data into the @theme. You only need to call this if you
 * lazy-loaded the theme or called b_theme_unload() before.
 *
 * Return value: %TRUE on success, FALSE otherwise
 **/
gboolean
b_theme_load (BTheme  *theme,
              GError **error)
{
  gboolean retval;

  g_return_val_if_fail (B_IS_THEME (theme), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  b_theme_unload (theme);

  if (b_object_get_filename (B_OBJECT (theme)))
    {
      retval = b_theme_parser_parse (theme, FALSE, error);
    }
  else
    {
      b_theme_load_from_scratch (theme);
      retval = TRUE;
    }

  theme->needs_blending = b_theme_needs_blending (theme);

  return retval;
}

/**
 * b_theme_unload:
 * @theme: a #BTheme object
 *
 * Frees all data of a BTheme except the meta information stored in
 * the header.
 **/
void
b_theme_unload (BTheme *theme)
{
  GList *list;

  g_return_if_fail (B_IS_THEME (theme));

  g_free (theme->bg_image);
  theme->bg_image = NULL;

  for (list = theme->overlays; list; list = list->next)
    {
      BOverlay *overlay = list->data;
      GList    *window;

      g_free (overlay->image);
      overlay->image = NULL;

      for (window = overlay->windows; window; window = window->next)
        g_free (window->data);

      g_list_free (overlay->windows);
      g_free (overlay);
    }

  g_list_free (theme->overlays);
  theme->overlays = NULL;

  theme->needs_blending = FALSE;
}

static void
b_theme_frame_boundary (BTheme     *theme,
                        BWindow    *window,
                        guchar      data,
                        BRectangle *bbox)
{
  if (! data)
    return;

  window += (data * theme->maxval) / 256;

  b_rectangle_union (&window->rect, bbox, bbox);
}

/**
 * b_theme_frame_diff_boundary
 * @theme:       a #BTheme
 * @prev_data:   data of the previous frame
 * @frame_data:  data of the current frame
 * @bbox:        returns bounding box
 *
 * Computes the bounding box of the difference image between two frames.
 *
 * Return value: %TRUE if the bounding box is not empty.
 **/
gboolean
b_theme_frame_diff_boundary (BTheme       *theme,
                             const guchar *prev_data,
                             const guchar *frame_data,
                             BRectangle   *bbox)
{
  GList *list;

  g_return_val_if_fail (B_IS_THEME (theme), FALSE);
  g_return_val_if_fail (bbox != NULL, FALSE);

  bbox->x = 0;
  bbox->y = 0;
  bbox->w = 0;
  bbox->h = 0;

  for (list = theme->overlays; list; list = list->next)
    {
      BOverlay *overlay = list->data;

      if (overlay->image)
        {
          GList *iter;

          for (iter = overlay->windows; iter; iter = iter->next)
            {
              BWindow *window = iter->data;
              guint    index  = window->column + window->row * theme->columns;

              if (prev_data && frame_data)
                {
                  if (frame_data[index] != prev_data[index])
                    {
                      b_theme_frame_boundary (theme,
                                              window, frame_data[index], bbox);
                      b_theme_frame_boundary (theme,
                                              window, prev_data[index], bbox);
                    }
                }
              else if (prev_data)
                {
                  b_theme_frame_boundary (theme,
                                          window, prev_data[index], bbox);
                }
              else if (frame_data)
                {
                  b_theme_frame_boundary (theme,
                                          window, frame_data[index], bbox);
                }
            }
        }
    }

  return (bbox->w > 0 && bbox->h > 0);
}

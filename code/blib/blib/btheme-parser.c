/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (C) 2002  The Blinkenlights Crew
 *                     Sven Neumann <sven@gimp.org>
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

#include <glib-object.h>

#include "btypes.h"
#include "bobject.h"
#include "bparser.h"
#include "btheme.h"
#include "btheme-parser.h"
#include "butils.h"


enum
{
  THEME = B_PARSER_STATE_USER,
  BACKGROUND,
  OVERLAY,
  GRID,
  GRID_WINDOW,
  SPAN,
  SPAN_WINDOW,
  WINDOW,
  FINISH
};

typedef struct
{
  BTheme   *theme;
  gchar    *dirname;
  gboolean  lazy;
  BOverlay *overlay;
  gint      dx, dy;
  gint      sx, sy;
  GList    *windows;
} ParserData;


static BParserState b_theme_start_element (BParserState   state,
                                           const gchar     *element_name,
                                           const gchar    **attribute_names,
                                           const gchar    **attribute_values,
                                           gpointer         user_data,
                                           GError         **error);
static BParserState b_theme_end_element   (BParserState   state,
                                           const gchar     *element_name,
                                           const gchar     *cdata,
                                           gsize            cdata_len,
                                           gpointer         user_data,
                                           GError         **error);
static void         b_theme_parse_header  (BTheme          *theme,
                                           const gchar    **attribute_names,
                                           const gchar    **attribute_values);
static void         b_theme_parse_image   (ParserData      *data,
                                           BOverlay        *overlay,
                                           const gchar    **attribute_names,
                                           const gchar    **attribute_values);
static void         b_theme_parse_grid    (ParserData      *data,
                                           const gchar    **attribute_names,
                                           const gchar    **attribute_values);
static void         b_theme_parse_window  (BWindow         *window,
                                           const gchar    **attribute_names,
                                           const gchar    **attribute_values);
static void    b_theme_overlay_add_window (BTheme          *theme,
                                           BOverlay        *overlay,
                                           const BWindow   *window);
static gboolean b_theme_window_validate   (BTheme          *theme,
                                           const BWindow   *window);


gboolean
b_theme_parser_parse (BTheme    *theme,
                      gboolean   lazy,
                      GError   **error)
{
  BParser     *parser;
  ParserData   data = { NULL };
  GIOChannel  *io;
  const gchar *filename;
  gboolean     retval;

  g_return_val_if_fail (theme != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  filename = b_object_get_filename (B_OBJECT (theme));
  g_return_val_if_fail (filename != NULL, FALSE);

  io = g_io_channel_new_file (filename, "r", error);
  if (!io)
    return FALSE;

  data.theme   = theme;
  data.dirname = g_path_get_dirname (filename);
  data.lazy    = lazy;

  parser = b_parser_new (b_theme_start_element, b_theme_end_element, &data);

  retval = b_parser_parse_io_channel (parser, io, TRUE, error);

  if (retval && b_parser_get_state (parser) != FINISH)
    {
      g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                   "This does not look like a Blinkenlights Theme");
      retval = FALSE;
    }

  g_io_channel_unref (io);

  b_parser_free (parser);

  g_free (data.dirname);

  if (retval &&
      (theme->rows     < 1 || theme->columns < 1   ||
       theme->channels < 1 ||
       theme->maxval   < 1 || theme->maxval  > 256 ||
       theme->width    < 1 || theme->height  < 1))
    {
      g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                   "Incorrect theme header");
      retval = FALSE;
    }

  return retval;
}

static BParserState
b_theme_start_element (BParserState   state,
                       const gchar   *element_name,
                       const gchar  **attribute_names,
                       const gchar  **attribute_values,
                       gpointer       user_data,
                       GError       **error)
{
  ParserData *data = (ParserData *) user_data;

  switch (state)
    {
    case B_PARSER_STATE_TOPLEVEL:
      if (strcmp (element_name, "blinkentheme") == 0)
        {
          b_theme_parse_header (data->theme,
                                attribute_names, attribute_values);
          return THEME;
        }
      break;

    case THEME:
      if (data->lazy)
        return B_PARSER_STATE_UNKNOWN;

      if (strcmp (element_name, "background") == 0)
        {
          b_theme_parse_image (data, NULL,
                               attribute_names, attribute_values);
          b_parse_coordinates (attribute_names, attribute_values,
                               &data->theme->bg_image_x,
                               &data->theme->bg_image_y);
          return BACKGROUND;
        }
      else if (strcmp (element_name, "overlay") == 0)
        {
          BOverlay *overlay = g_new0 (BOverlay, 1);

          b_theme_parse_image (data, overlay,
                               attribute_names, attribute_values);

          data->overlay = overlay;
          return OVERLAY;
        }
      break;

    case OVERLAY:
      if (strcmp (element_name, "grid") == 0)
        {
          b_theme_parse_grid (data, attribute_names, attribute_values);
          return GRID;
        }
      else if (strcmp (element_name, "span") == 0)
        {
          b_theme_parse_grid (data, attribute_names, attribute_values);
          return SPAN;
        }
      else if (strcmp (element_name, "window") == 0)
        {
          BWindow  window;

          b_theme_parse_window (&window, attribute_names, attribute_values);

          if (b_theme_window_validate (data->theme, &window))
            b_theme_overlay_add_window (data->theme, data->overlay, &window);

          return WINDOW;
        }

      break;

    case GRID:
    case SPAN:
      if (strcmp (element_name, "window") == 0)
        {
          BWindow  window;

          b_theme_parse_window (&window, attribute_names, attribute_values);
          if (b_theme_window_validate (data->theme, &window))
            data->windows = g_list_append (data->windows,
                                           g_memdup (&window,
                                                     sizeof (BWindow)));

          return state == GRID ? GRID_WINDOW : SPAN_WINDOW;
        }
      break;

    case GRID_WINDOW:
    case SPAN_WINDOW:
    case WINDOW:
      g_printerr ("window element should be empty");
      break;

    default:
      break;
    }

  return B_PARSER_STATE_UNKNOWN;
}

static BParserState
b_theme_end_element (BParserState   state,
                     const gchar   *element_name,
                     const gchar   *cdata,
                     gsize          cdata_len,
                     gpointer       user_data,
                     GError       **error)
{
  ParserData *data = (ParserData *) user_data;
  GList      *list;

  switch (state)
    {
    case THEME:
      data->theme->overlays = g_list_reverse (data->theme->overlays);
      return FINISH;

    case BACKGROUND:
      return THEME;

    case OVERLAY:
      g_return_val_if_fail (data->overlay != NULL, THEME);

      if (data->overlay->windows)
        {
          data->overlay->windows = g_list_reverse (data->overlay->windows);
          data->theme->overlays = g_list_prepend (data->theme->overlays,
                                                  data->overlay);
        }
      else
        {
          g_printerr ("no windows defined in overlay, skipping");

          g_free (data->overlay->image);
          g_free (data->overlay);
        }
      data->overlay = NULL;
      return THEME;

    case GRID:
    case SPAN:
      for (list = data->windows; list; list = list->next)
        {
          BWindow *template = list->data;
          gint     row, col;

          template->column = 0;
          if (state == GRID)
            template->row = 0;

          for (row = template->row; row < data->theme->rows; row++)
            {
              for (col = 0; col < data->theme->columns; col++)
                {
                  BWindow  window = *template;

                  window.row    = row;
                  window.column = col;
                  window.src_x  += col * data->sx;
                  window.rect.x += col * data->dx;

#if 0
                  g_print ("window %d,%d (%d) is %dx%d @ %d,%d src=%d,%d\n",
                           window.column, window.row, window.value,
                           window.rect.w, window.rect.h,
                           window.rect.x, window.rect.y,
                           window.src_x,  window.src_y);
#endif

                  b_theme_overlay_add_window (data->theme, data->overlay,
                                              &window);
                }

              if (state == SPAN)
                break;

              template->src_y  += data->sy;
              template->rect.y += data->dy;
            }

          g_free (template);
        }

      g_list_free (data->windows);
      data->windows = NULL;
      return OVERLAY;

    case GRID_WINDOW:
      return GRID;

    case SPAN_WINDOW:
      return SPAN;

    case WINDOW:
      return OVERLAY;

    default:
      break;
    }

  return B_PARSER_STATE_UNKNOWN;
}

static void
b_theme_parse_header (BTheme        *theme,
                      const gchar  **attribute_names,
                      const gchar  **attribute_values)
{
  const gchar **name;
  const gchar **value;

  if (theme->type)
    {
      g_free (theme->type);
      theme->type = NULL;
    }

  theme->rows     = 0;
  theme->columns  = 0;
  theme->channels = 1;
  theme->maxval   = 1;
  theme->width    = 0;
  theme->height   = 0;

  for (name = attribute_names, value = attribute_values;
       *name && *value;
       name++, value++)
    {
      if (strcmp (*name, "title") == 0)
        b_object_set_name (B_OBJECT (theme), *value);
      if (strcmp (*name, "type") == 0)
        theme->type = g_strdup (*value);
      if (strcmp (*name, "rows") == 0)
        b_parse_int (*value, &theme->rows);
      if (strcmp (*name, "columns") == 0)
        b_parse_int (*value, &theme->columns);
      if (strcmp (*name, "channels") == 0)
        b_parse_int (*value, &theme->channels);
      if (strcmp (*name, "maxval") == 0)
        b_parse_int (*value, &theme->maxval);
      if (strcmp (*name, "width") == 0)
        b_parse_int (*value, &theme->width);
      if (strcmp (*name, "height") == 0)
        b_parse_int (*value, &theme->height);
    }
}

static void
b_theme_parse_image (ParserData   *data,
                     BOverlay     *overlay,
                     const gchar **attribute_names,
                     const gchar **attribute_values)
{
  const gchar **name;
  const gchar **value;
  gchar       **image;
  BColor       *color;

  image = overlay ? &overlay->image : &data->theme->bg_image;

  for (name = attribute_names, value = attribute_values;
       *name && *value;
       name++, value++)
    {
      if (!*image && strcmp (*name, "image") == 0)
        *image = g_build_filename (data->dirname, *value, NULL);
    }

  color = overlay ? &overlay->color : &data->theme->bg_color;

  if (overlay)
    color->r = color->g = color->b = 0xFF;
  else
    color->r = color->g = color->b = 0x00;

  color->a = 0xFF;

  b_parse_color (attribute_names, attribute_values, color);
}

static void
b_theme_parse_grid (ParserData   *data,
                    const gchar **attribute_names,
                    const gchar **attribute_values)
{
  const gchar **name;
  const gchar **value;

  data->dx = data->dy = data->sx = data->sy = 0;

  for (name = attribute_names, value = attribute_values;
       *name && *value;
       name++, value++)
    {
      if (strcmp (*name, "dx") == 0)
        b_parse_int (*value, &data->dx);
      if (strcmp (*name, "dy") == 0)
        b_parse_int (*value, &data->dy);
      if (strcmp (*name, "sx") == 0)
        b_parse_int (*value, &data->sx);
      if (strcmp (*name, "sy") == 0)
        b_parse_int (*value, &data->sy);
    }
}

static void
b_theme_parse_window (BWindow      *window,
                      const gchar **attribute_names,
                      const gchar **attribute_values)
{
  const gchar **name;
  const gchar **value;

  window->value  = B_WINDOW_VALUE_ALL;
  window->row    = window->column = 0;
  window->src_x  = window->src_y  = 0;
  window->rect.x = window->rect.y = window->rect.w = window->rect.h = 0;

  for (name = attribute_names, value = attribute_values;
       *name && *value;
       name++, value++)
    {
      if (strcmp (*name, "value") == 0 && strcmp (*value, "all"))
        b_parse_int (*value, &window->value);
      if (strcmp (*name, "row") == 0)
        b_parse_int (*value, &window->row);
      if (strcmp (*name, "column") == 0)
        b_parse_int (*value, &window->column);
      if (strcmp (*name, "src-x") == 0)
        b_parse_int (*value, &window->src_x);
      if (strcmp (*name, "src-y") == 0)
        b_parse_int (*value, &window->src_y);
    }

  b_parse_rectangle (attribute_names, attribute_values, &window->rect);
}

static void
b_theme_overlay_add_window (BTheme        *theme,
                            BOverlay      *overlay,
                            const BWindow *window)
{
  BWindow *windows;
  GList   *list;
  gint     i;

  for (list = overlay->windows; list; list = list->next)
    {
      windows = list->data;

      if (windows->row == window->row && windows->column == window->column)
        break;
    }

  if (!list)
    {
      windows = g_new (BWindow, theme->maxval);
      for (i = 0; i < theme->maxval; i++)
        {
          windows[i] = *window;
          windows[i].value = B_WINDOW_VALUE_ALL;
        }
      overlay->windows = g_list_prepend (overlay->windows, windows);
    }

  if (window->value != B_WINDOW_VALUE_ALL)
    {
      windows[window->value - 1] = *window;
    }
}

static gboolean
b_theme_window_validate (BTheme        *theme,
                         const BWindow *window)
{
  if (window
      &&
      (window->value != B_WINDOW_VALUE_ALL &&
      (window->value  < 1 || window->value  > theme->maxval))
      &&
      (window->row    < 0 || window->row    >= theme->rows  ||
       window->column < 0 || window->column >= theme->columns))
    {
      g_printerr ("Invalid window, skipping");
      return FALSE;
    }

  return TRUE;
}

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

#ifndef __B_THEME_H__
#define __B_THEME_H__

#define B_WINDOW_VALUE_ALL 0

G_BEGIN_DECLS

typedef  struct _BWindow   BWindow;
typedef  struct _BOverlay  BOverlay;

struct _BWindow
{
  gint        value;
  gint        row;
  gint        column;
  gint        src_x;
  gint        src_y;
  BRectangle  rect;
};

struct _BOverlay
{
  gchar       *image;
  BColor       color;

  GList       *windows;
};


#define B_TYPE_THEME            (b_theme_get_type ())
#define B_THEME(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_THEME, BTheme))
#define B_THEME_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_THEME, BThemeClass))
#define B_IS_THEME(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_THEME))
#define B_IS_THEME_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_THEME))
#define B_THEME_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), B_TYPE_THEME, BThemeClass))


typedef struct _BThemeClass BThemeClass;

struct _BThemeClass
{
  BObjectClass  parent_class;
};

struct _BTheme
{
  BObjectClass   parent_class;

  gchar         *type;

  gint           rows;
  gint           columns;
  gint           channels;
  gint           maxval;

  gint           width;
  gint           height;

  BColor         bg_color;
  gchar         *bg_image;
  gint           bg_image_x;
  gint           bg_image_y;

  GList         *overlays;

  gboolean       needs_blending;  /* for optimizations */
};


GType      b_theme_get_type            (void) G_GNUC_CONST;

BTheme   * b_theme_new_from_file       (const gchar  *filename,
                                        gboolean      lazy_load,
                                        GError      **error);
BTheme   * b_theme_new_from_scratch    (const gchar  *title,
                                        const gchar  *type,
                                        gint          rows,
                                        gint          columns,
                                        gint          channels,
                                        gint          maxval,
                                        gint          width,
                                        gint          height);

gboolean   b_theme_load                (BTheme       *theme,
                                        GError      **error);
void       b_theme_unload              (BTheme       *theme);

gboolean   b_theme_frame_diff_boundary (BTheme       *theme,
                                        const guchar *prev_data,
                                        const guchar *frame_data,
                                        BRectangle   *bbox);


G_END_DECLS

#endif  /* __B_THEME_H__ */

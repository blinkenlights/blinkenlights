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

#ifndef __B_THEMES_H__
#define __B_THEMES_H__

G_BEGIN_DECLS

typedef enum
{
  B_THEMES_QUERY_NONE    = 0,
  B_THEMES_QUERY_NAME    = 1 << 0,
  B_THEMES_QUERY_TYPE    = 1 << 1,
  B_THEMES_QUERY_ROWS    = 1 << 2,
  B_THEMES_QUERY_COLUMNS = 1 << 3,
  B_THEMES_QUERY_WIDTH   = 1 << 4,
  B_THEMES_QUERY_HEIGHT  = 1 << 5
} BThemesQueryFlags;

struct _BThemesQuery
{
  BThemesQueryFlags  flags;

  const gchar       *name;
  const gchar       *type;
  gint               rows;
  gint               columns;
  gint               width;
  gint               height;
};

typedef gboolean (* BThemesForeachFunc) (BTheme   *theme,
					 gpointer  callback_data);


/* searches a theme in the theme path */
GList * b_themes_query   (const gchar        *themepath,
			  BThemesQuery       *query); /* may be NULL */

/* calls callback for each theme with the short name and the theme */
void    b_themes_foreach (const gchar        *themepath,  /* may be NULL */
                          BThemesForeachFunc  callback,
                          gpointer            callback_data);


#ifndef B_DISABLE_DEPRECATED

/* some wrappers that provide the old (pre 1.0) API, don't use  */
void     b_themes_foreach_theme (const gchar  *themepath,  /* may be NULL */
				 GHFunc        callback,
				 gpointer      callback_data);
BTheme * b_themes_lookup_theme  (const gchar  *name,
				 const gchar  *themepath,  /* may be NULL */
				 GError      **error);

#endif /* B_DISABLE_DEPRECATED */


G_END_DECLS

#endif  /* __B_THEMES_H__ */

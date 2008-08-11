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
#include "btheme.h"
#include "bthemes.h"


typedef struct
{
  BThemesQuery *query;
  GList        *list;
} BThemesQueryData;


static const gchar *
b_themes_get_path (void)
{
  const gchar *env = g_getenv ("B_THEME_PATH");

  if (env)
    return env;

  return THEMEPATH;
}

static gboolean
b_themes_match_query (BTheme   *theme,
                      gpointer  callback_data)
{
   BThemesQueryData *data  = callback_data;
   BThemesQuery     *query = data->query;

   if ((query->flags & B_THEMES_QUERY_ROWS) && 
       (query->rows != theme->rows))
     return TRUE;
   if ((query->flags & B_THEMES_QUERY_COLUMNS) && 
       (query->columns != theme->columns))
     return TRUE;
   if ((query->flags & B_THEMES_QUERY_WIDTH) && 
       (query->width != theme->width))
     return TRUE;
   if ((query->flags & B_THEMES_QUERY_HEIGHT) && 
       (query->height != theme->height))
     return TRUE;

   if ((query->flags & B_THEMES_QUERY_TYPE))
     {
       if (query->type)
	 {
	   if (theme->type == NULL || strcmp (query->type, theme->type))
	     return TRUE;
	 }
       else if (theme->type)
	 return TRUE;
     }

   if ((query->flags & B_THEMES_QUERY_NAME))
     {
       const gchar *name = b_object_get_name (B_OBJECT (theme));

       if (query->name)
	 {
	   if (name == NULL || strcmp (query->name, name))
	     return TRUE;
	 }
       else if (name)
	 return TRUE;
     }

   g_object_ref (G_OBJECT (theme));
   data->list = g_list_prepend (data->list, theme); 
   
   return TRUE;
}

/**
 * b_themes_query:
 * @themepath: a colon-separated list of directories to search or %NULL to
 * use the default path
 * @query: pointer to a #BThemesQuery
 * 
 * Looks for themes as defined by @query. If @themepath is not
 * specified the default path is used. The default path can be overridden
 * by setting the environment variable B_THEME_PATH.
 *
 * Each theme that matches the @query is lazy-loaded.
 *
 * Return value: a #GList of newly allocated, lazy-loaded #BTheme objects
 * or %NULL if no matching theme was found
 **/
GList *
b_themes_query (const gchar   *themepath,
		BThemesQuery  *query)
{
  BThemesQueryData  query_data;

  g_return_val_if_fail (query != NULL, FALSE);

  query_data.query = query;
  query_data.list  = NULL;

  b_themes_foreach (themepath, b_themes_match_query, &query_data);
  
  return g_list_reverse (query_data.list);
}

/**
 * b_themes_foreach
 * @themepath: a colon-separated list of directories to search or %NULL to
 * use the default path
 * @callback: a function to call for each theme
 * @callback_data: data to pass to the @callback
 *
 * This function iterates over all themes in the @themepath, lazy-loads them,
 * runs @callback on the theme and unrefs it again.  The iteration is stopped
 * if a @callback returns %FALSE.
 *
 * If @themepath is not specified, the default path is used. The
 * default path can be overridden by setting the environment variable
 * B_THEME_PATH.
 **/
void
b_themes_foreach (const gchar        *themepath,
                  BThemesForeachFunc  callback,
                  gpointer            callback_data)
{
  BTheme    *theme;
  gchar    **dirs;
  gint       i;
  gboolean   cont = TRUE;

  g_return_if_fail (callback != NULL);

  if (!themepath)
    themepath = b_themes_get_path ();

  dirs = g_strsplit (themepath, G_SEARCHPATH_SEPARATOR_S, 12);

  for (i = 0; cont && dirs[i]; i++)
    {
      const gchar *dirname  = dirs[i];
      const gchar *name;
      GDir        *dir;
 
      dir = g_dir_open (dirname, 0, NULL);
      if (!dir)
        continue;

      while (cont && (name = g_dir_read_name (dir)))
        {
          gchar *filename = g_build_filename (dirname, name, NULL);

          if (g_file_test (filename, G_FILE_TEST_IS_REGULAR))
            {
              theme = b_theme_new_from_file (filename, TRUE, NULL);
              if (theme)
                {
                  cont = callback (theme, callback_data);
                  g_object_unref (theme);
                }
            }
          
          g_free (filename);
        }

      g_dir_close (dir);
    }

  g_strfreev (dirs);
}


/* some wrappers that provide the old (pre 1.0) API, don't use  */

typedef struct
{
  GHFunc    callback;
  gpointer  data;
} WrapperData;

static gboolean
wrapper (BTheme   *theme,
	 gpointer  callback_data)
{
  WrapperData *wrapper_data = callback_data;
  gchar       *basename;
  gchar       *suffix;

  basename = g_path_get_basename (b_object_get_filename (B_OBJECT (theme)));

  if ((suffix = g_strrstr (basename, ".xml")) != NULL)
    suffix[0] = '\0';

  wrapper_data->callback (basename, theme, wrapper_data->data);

  g_free (basename);

  return TRUE;
}

/**
 * b_themes_foreach_theme:
 * @themepath: 
 * @callback: 
 * @callback_data: 
 * 
 * Shouldn't be used in new code, use b_themes_foreach() instead.
 **/
void
b_themes_foreach_theme (const gchar  *themepath,
			GHFunc        callback,
			gpointer      callback_data)
{
  WrapperData wrapper_data = { callback, callback_data };

  g_return_if_fail (callback != NULL);
  
  b_themes_foreach (themepath, wrapper, &wrapper_data);
}

/**
 * b_themes_lookup_theme:
 * @name: 
 * @themepath: 
 * @error: 
 * 
 * Shouldn't be used in new code, use b_themes_query() instead.
 * 
 * Return value: 
 **/
BTheme *
b_themes_lookup_theme (const gchar  *name,
                       const gchar  *themepath,
                       GError      **error)
{
  BTheme  *theme = NULL;
  gchar  **dirs;
  gchar   *filename;
  gint     i;

  g_return_val_if_fail (name != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (!themepath)
    themepath = b_themes_get_path ();

  filename = g_strconcat (name, ".xml", NULL);

  dirs = g_strsplit (themepath, G_SEARCHPATH_SEPARATOR_S, 12);

  for (i = 0; !theme && dirs[i]; i++)
    {
      const gchar *dirname  = dirs[i];
      gchar       *fullname = g_build_filename (dirname, filename, NULL);

      if (g_file_test (fullname, G_FILE_TEST_IS_REGULAR))
        {
          theme = b_theme_new_from_file (fullname, TRUE, NULL);
        }
      else /* try w/o the suffix */
        {
          g_free (fullname);
          fullname = g_build_filename (dirname, name, NULL);

          if (g_file_test (fullname, G_FILE_TEST_IS_REGULAR))
            theme = b_theme_new_from_file (fullname, TRUE, NULL);
        }

      g_free (fullname);
    }

  g_strfreev (dirs);

  g_free (filename);

  if (!theme)
    g_set_error (error, 0, 0,
                 "No theme of this name found in '%s'", themepath);

  return theme;
}

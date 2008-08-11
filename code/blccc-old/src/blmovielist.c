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

#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <dirent.h>

#include <gtk/gtk.h>

#include "bltypes.h"
#include "blmovie.h"
#include "blmovielist.h"


static void   bl_movie_list_class_init  (BlMovieListClass *class);
static void   bl_movie_list_init        (BlMovieList      *list);
static void   bl_movie_list_destroy     (GtkObject        *object);


static GtkObjectClass *parent_class = NULL;


GtkType
bl_movie_list_get_type (void)
{
  static GtkType list_type = 0;

  if (!list_type)
    {
      GtkTypeInfo list_info =
      {
	"BlMovieList",
	sizeof (BlMovieList),
	sizeof (BlMovieListClass),
	(GtkClassInitFunc) bl_movie_list_class_init,
	(GtkObjectInitFunc) bl_movie_list_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL
      };

      list_type = gtk_type_unique (gtk_object_get_type (), &list_info);
    }
  
  return list_type;
}

static void
bl_movie_list_class_init (BlMovieListClass *class)
{
  GtkObjectClass *object_class;

  parent_class = gtk_type_class (gtk_object_get_type ());
  object_class = GTK_OBJECT_CLASS (class);

  object_class->destroy = bl_movie_list_destroy;
}

static void
bl_movie_list_init (BlMovieList *list)
{
  list->dirname = NULL;
  list->movies = NULL;
}

static void
bl_movie_list_destroy (GtkObject *object)
{
  BlMovieList *list;

  list = BL_MOVIE_LIST (object);

  g_free (list->dirname);

  g_list_foreach (list->movies, (GFunc) gtk_object_unref, NULL);
  g_list_free (list->movies);

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

BlMovieList *
bl_movie_list_new (const gchar *dirname)
{
  BlMovieList   *list;
  
  g_return_val_if_fail (dirname != NULL, NULL);

  list = BL_MOVIE_LIST (gtk_object_new (BL_TYPE_MOVIE_LIST, NULL));
  list->dirname = g_strdup (dirname);

  bl_movie_list_reload (list);
  
  return list;
}

void
bl_movie_list_reload (BlMovieList *list)
{
  BlMovie       *movie;
  DIR           *dir;
  struct dirent *file;

  g_return_if_fail (list != NULL);
  g_return_if_fail (BL_IS_MOVIE_LIST (list));

  g_list_foreach (list->movies, (GFunc) gtk_object_unref, NULL);
  g_list_free (list->movies);
  list->movies = NULL;
  
  dir = opendir (list->dirname);
  if (!dir)
    {
      g_printerr ("Couldn't open directory '%s': %s\n",
                  list->dirname, g_strerror (errno));
      return;
    }

  while ((file = readdir (dir)) != NULL)
    {
      movie = bl_movie_new (file->d_name);
      if (movie)
        {
          gtk_object_ref  (GTK_OBJECT (movie));
          gtk_object_sink (GTK_OBJECT (movie));                          
          list->movies = g_list_prepend (list->movies, movie);
        }
    }

  closedir (dir);

  list->movies = g_list_reverse (list->movies);
}

gint
bl_movie_list_get_num_movies (BlMovieList *list)
{
  g_return_val_if_fail (list != NULL, 0);
  g_return_val_if_fail (BL_IS_MOVIE_LIST (list), 0);
  
  return g_list_length (list->movies);
}

void
bl_movie_list_foreach_movie (BlMovieList *list,
                             GFunc        func,
                             gpointer     data)
{
  g_return_if_fail (list != NULL);
  g_return_if_fail (BL_IS_MOVIE_LIST (list));
  
  g_return_if_fail (func != NULL);

  g_list_foreach (list->movies, func, data);
}

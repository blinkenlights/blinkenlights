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
#include "blmovie.h"
#include "blplaylist.h"


static void   bl_play_list_class_init  (BlPlayListClass *class);
static void   bl_play_list_init        (BlPlayList      *list);


static GtkCListClass *parent_class = NULL;


GtkType
bl_play_list_get_type (void)
{
  static GtkType list_type = 0;

  if (!list_type)
    {
      GtkTypeInfo list_info =
      {
	"BlPlayList",
	sizeof (BlPlayList),
	sizeof (BlPlayListClass),
	(GtkClassInitFunc) bl_play_list_class_init,
	(GtkObjectInitFunc) bl_play_list_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL
      };

      list_type = gtk_type_unique (gtk_clist_get_type (), &list_info);
    }
  
  return list_type;
}

static void
bl_play_list_class_init (BlPlayListClass *class)
{
  parent_class = gtk_type_class (gtk_clist_get_type ());
}

static void
bl_play_list_init (BlPlayList *list)
{
  gchar *titles[3];

  titles[0] = "Filename";
  titles[1] = "Description                                   ";
  titles[2] = "Duration";

  gtk_clist_construct (GTK_CLIST (list), 3, titles);

  gtk_clist_set_reorderable (GTK_CLIST (list), TRUE);
  gtk_clist_set_selection_mode (GTK_CLIST (list), GTK_SELECTION_EXTENDED);
  gtk_clist_set_column_justification (GTK_CLIST (list), 2, GTK_JUSTIFY_RIGHT);
}

GtkWidget *
bl_play_list_new (void)
{
  return GTK_WIDGET (gtk_object_new (BL_TYPE_PLAY_LIST, NULL));
}

void
bl_play_list_append_movie (BlPlayList *playlist,
                           BlMovie    *movie)
{
  gint   row;
  gchar  buf[16];
  gchar *text[3];

  g_return_if_fail (playlist != NULL);
  g_return_if_fail (BL_IS_PLAY_LIST (playlist));

  g_return_if_fail (movie != NULL);
  g_return_if_fail (BL_IS_MOVIE (movie));

  if (!bl_movie_load (movie))
    {
      g_message ("Couldn't load movie file '%s'. "
                 "Can't add it to playlist.", movie->filename);
      return;
    }

  gtk_object_ref (GTK_OBJECT (movie));
  gtk_object_sink (GTK_OBJECT (movie));

  g_snprintf (buf, sizeof (buf), "%.2f s", 
              (gdouble) movie->duration / 1000.0);

  text[0] = movie->filename;
  text[1] = movie->description;
  text[2] = buf;

  row = gtk_clist_append (GTK_CLIST (playlist), text);

  gtk_clist_set_row_data_full (GTK_CLIST (playlist), row, movie,
                               (GDestroyNotify) gtk_object_unref);
}

void          
bl_play_list_remove_selected (BlPlayList *playlist)
{
  GtkCList *clist;
  GList    *list;
  GList    *remove = NULL;
  gint      row;

  g_return_if_fail (playlist != NULL);
  g_return_if_fail (BL_IS_PLAY_LIST (playlist));

  clist = GTK_CLIST (playlist);

  for (list = clist->row_list; list; list = g_list_next (list))
    {
      if (GTK_CLIST_ROW (list)->state == GTK_STATE_SELECTED)
        {
          remove = g_list_prepend (remove, GTK_CLIST_ROW (list)->data);
        }
    }

  for (list = remove; list; list = g_list_next (list))
    {
      row = gtk_clist_find_row_from_data (clist, list->data);
      gtk_clist_remove (clist, row);
    }

  g_list_free (remove);
}

BlMovie *
bl_play_list_get_next_movie (BlPlayList *playlist)
{
  gpointer movie;

  g_return_val_if_fail (playlist != NULL, NULL);
  g_return_val_if_fail (BL_IS_PLAY_LIST (playlist), NULL);

  movie = gtk_clist_get_row_data (GTK_CLIST (playlist), 0);

  if (movie)
    {
      gtk_object_ref (GTK_OBJECT (movie));
      gtk_clist_remove (GTK_CLIST (playlist), 0);
      bl_play_list_append_movie (playlist, BL_MOVIE (movie));

      return BL_MOVIE (movie);
    }

  return NULL;
}

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

#include "blccc.h"
#include "bltypes.h"
#include "blmovie.h"
#include "blmovielist.h"
#include "blmovieview.h"
#include "blplaylist.h"
#include "blpreview.h"
#include "bltheater.h"
#include "main-window.h"


static const gchar *default_msg  = "XXCCC: Blinkenlights Chaos Control Center";
static guint        msg_timeout  = 0;
static GtkWidget   *msg_label    = NULL;

static GtkWidget   *playlist     = NULL;
static GtkWidget   *start_button = NULL;


static void
clist_append_movie (BlMovie  *movie,
                    GtkCList *clist)
{
  gchar  buf[16];
  gchar *text[3];
  gint   row;

  g_snprintf (buf, sizeof (buf), "%.2f s", 
              (gdouble) movie->duration / 1000.0);
  
  text[0] = movie->filename;
  text[1] = movie->description;
  text[2] = buf;

  row = gtk_clist_append (clist, text);
  gtk_clist_set_row_data (clist, row, movie);
}

static void
clist_reload (GtkWidget *widget,
              GtkCList  *clist)
{
  BlMovieList *list;

  list = BL_MOVIE_LIST (gtk_object_get_data (GTK_OBJECT (clist), "list"));

  gtk_clist_freeze (GTK_CLIST (clist));
  gtk_clist_clear (clist);
  bl_movie_list_reload (list);
  bl_movie_list_foreach_movie (list, (GFunc) clist_append_movie, clist);
  gtk_clist_thaw (GTK_CLIST (clist));
}

static void
add_selected_to_playlist (GtkWidget *widget,
                          gpointer   data)
{
  BlMovie    *movie;
  GtkCList   *clist;
  GList      *rows;

  clist = GTK_CLIST (data);

  for (rows = clist->row_list; rows; rows = g_list_next (rows))
    {
      if (GTK_CLIST_ROW (rows)->state == GTK_STATE_SELECTED)
        {
          movie = (BlMovie *) GTK_CLIST_ROW (rows)->data;
          bl_play_list_append_movie (BL_PLAY_LIST (playlist), movie);
        }
    }
}

static void
dirlist_select_row (GtkCList *clist,
                    gint      row,
                    gint      column,
                    GdkEvent *event,
                    gpointer  data)
{
  gpointer movie;

  movie = gtk_clist_get_row_data (clist, row);
  
  bl_movie_view_set_movie (BL_MOVIE_VIEW (data), 
                           movie ? BL_MOVIE (movie) : NULL);
}

static void
movie_started_callback (BlTheater *theater,
                        BlMovie   *movie,
                        gpointer   data)
{
  GtkLabel *label = GTK_LABEL (data);

  gtk_label_set_text (label, movie ? movie->filename : NULL);
}

static void
movie_finished_callback (BlTheater *theater,
                         BlMovie   *movie,
                         gpointer   data)
{
  BlMovie *next;

  next = bl_play_list_get_next_movie (BL_PLAY_LIST (playlist));
  
  if (next)
    bl_theater_set_movie (theater, next);
  else
    movie_started_callback (theater, NULL, data);

  gtk_widget_set_sensitive (start_button, next == NULL);
}

static void
start_callback (GtkWidget *widget,
                BlTheater *theater)
{
  movie_finished_callback (theater, NULL, NULL);
}

static gboolean
reset_message (gpointer data)
{
  gtk_label_set_text (GTK_LABEL (data), default_msg);
  return FALSE;
}

static void
show_message (const gchar    *log_domain,
              GLogLevelFlags  log_level,
              const gchar    *message,
              gpointer        data)
{
  if (msg_timeout)
    {
      g_source_remove (msg_timeout);
      msg_timeout = 0;
    }

  g_printerr ("BLCCC-Message: %s\n", message);
  gtk_label_set_text (GTK_LABEL (data), message);

  msg_timeout = g_timeout_add (3000, 
                               (GSourceFunc) reset_message, data);
}

void
main_window_create (BlTheater   *theater,
                    BlMovieList *list)
{
  GtkWidget *window;
  GtkWidget *main_vbox;
  GtkWidget *table;
  GtkWidget *now;
  GtkWidget *frame;
  GtkWidget *preview;
  GtkWidget *view;
  GtkWidget *vbox;
  GtkWidget *scroll;
  GtkWidget *clist;
  GtkWidget *button;
  GtkWidget *arrow;
  GtkWidget *label;
  gchar     *titles[2];
  guint      log_handler;

  g_return_if_fail (list != NULL);
  g_return_if_fail (BL_IS_MOVIE_LIST (list));

  g_return_if_fail (theater != NULL);
  g_return_if_fail (BL_IS_THEATER (theater));

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window),
                        "BlinkenLights Chaos Control Center");
  gtk_widget_set_usize (GTK_WIDGET (window), 800, 600);
  gtk_container_border_width (GTK_CONTAINER (window), 2);

  gtk_signal_connect (GTK_OBJECT (window), "destroy",
                      GTK_SIGNAL_FUNC (gtk_main_quit),
                      NULL);
  gtk_signal_connect (GTK_OBJECT (window), "delete_event",
                      GTK_SIGNAL_FUNC (gtk_widget_destroy),
                      NULL);  

  gtk_widget_show (window);

  main_vbox = gtk_vbox_new (FALSE, 4);
  gtk_container_add (GTK_CONTAINER (window), main_vbox);
  gtk_widget_show (main_vbox);

  table = gtk_table_new (4, 4, FALSE);
  gtk_table_set_col_spacings (GTK_TABLE (table), 4);
  gtk_table_set_row_spacings (GTK_TABLE (table), 4);
  gtk_box_pack_start (GTK_BOX (main_vbox), table, TRUE, TRUE, 0);
  gtk_widget_show (table);

  /* the playlist column */

  frame = gtk_aspect_frame_new ("Now Showing",
                                0.5, 0.5, ASPECT_RATIO, FALSE);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_table_attach (GTK_TABLE (table), frame, 0, 1, 0, 1,
                    GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
  gtk_widget_show (frame);

  now = bl_preview_new ();
  gtk_container_add (GTK_CONTAINER (frame), now);
  gtk_widget_show (now);

  bl_theater_set_preview (theater, BL_PREVIEW (now));

  vbox = gtk_vbox_new (FALSE, 2);
  gtk_table_attach (GTK_TABLE (table), vbox, 0, 1, 1, 2,
                    GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
  gtk_widget_show (vbox);

  start_button = gtk_button_new_with_label ("Start");
  gtk_box_pack_start (GTK_BOX (vbox), start_button, FALSE, FALSE, 0);
  gtk_widget_show (start_button);

  gtk_signal_connect (GTK_OBJECT (start_button), "clicked",
                      GTK_SIGNAL_FUNC (start_callback),
                      theater);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_start (GTK_BOX (vbox), frame, TRUE, TRUE, 0);
  gtk_widget_show (frame);

  label = gtk_label_new (NULL);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_container_add (GTK_CONTAINER (frame), label);
  gtk_widget_show (label);

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_table_attach_defaults (GTK_TABLE (table), scroll, 0, 1, 2, 3);
  gtk_widget_show (scroll);

  playlist = bl_play_list_new ();
  gtk_container_add (GTK_CONTAINER (scroll), playlist);
  gtk_widget_show (playlist);

  gtk_signal_connect (GTK_OBJECT (theater), "movie_started",
                      GTK_SIGNAL_FUNC (movie_started_callback),
                      label);
  gtk_signal_connect (GTK_OBJECT (theater), "movie_finished",
                      GTK_SIGNAL_FUNC (movie_finished_callback),
                      label);

  button = gtk_button_new_with_label ("Remove from Playlist");
  gtk_signal_connect_object (GTK_OBJECT (button), "clicked",
                             GTK_SIGNAL_FUNC (bl_play_list_remove_selected),
                             GTK_OBJECT (playlist));
  gtk_table_attach (GTK_TABLE (table), button, 0, 1, 3, 4,
                    GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
  gtk_widget_show (button);


  /* the dirlist column */

  frame = gtk_aspect_frame_new ("File Preview",
                                0.5, 0.5, ASPECT_RATIO, FALSE);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_table_attach (GTK_TABLE (table), frame, 2, 3, 0, 1,
                    GTK_FILL | GTK_EXPAND, GTK_FILL | GTK_SHRINK, 0, 0);
  gtk_widget_show (frame);

  preview = bl_preview_new ();
  gtk_widget_set_usize (GTK_WIDGET (preview), WIDTH * 10, HEIGHT * 20);
  gtk_container_add (GTK_CONTAINER (frame), preview);
  gtk_widget_show (preview);

  view = bl_movie_view_new (BL_PREVIEW (preview));
  gtk_table_attach (GTK_TABLE (table), view, 2, 3, 1, 2,
                    GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
  gtk_widget_show (view);

  scroll = gtk_scrolled_window_new (NULL, NULL);
  gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scroll),
                                  GTK_POLICY_AUTOMATIC, GTK_POLICY_ALWAYS);
  gtk_table_attach_defaults (GTK_TABLE (table), scroll, 2, 3, 2, 3);
  gtk_widget_show (scroll);

  titles[0] = "Filename";
  titles[1] = "Description                                   ";
  titles[2] = "Duration";

  clist = gtk_clist_new_with_titles (3, titles);
  gtk_clist_set_column_justification (GTK_CLIST (clist), 2, GTK_JUSTIFY_RIGHT);
  gtk_container_add (GTK_CONTAINER (scroll), clist);
  gtk_widget_show (clist);

  gtk_object_set_data (GTK_OBJECT (clist), "list", list);

  gtk_clist_freeze (GTK_CLIST (clist));
  bl_movie_list_foreach_movie (list, (GFunc) clist_append_movie, clist);
  gtk_clist_thaw (GTK_CLIST (clist));

  gtk_signal_connect (GTK_OBJECT (clist), "select_row",
                      GTK_SIGNAL_FUNC (dirlist_select_row),
                      view);

  gtk_clist_set_reorderable (GTK_CLIST (clist), FALSE);
  gtk_clist_set_selection_mode (GTK_CLIST (clist), GTK_SELECTION_EXTENDED);

  button = gtk_button_new_with_label ("Reload Directory");
  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (clist_reload),
                      clist);
  gtk_table_attach (GTK_TABLE (table), button, 2, 3, 3, 4,
                    GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_SHRINK, 0, 0);
  gtk_widget_show (button);


  /* the button column */

  button = gtk_button_new ();
  gtk_table_attach (GTK_TABLE (table), button, 1, 2, 2, 3,
                    GTK_FILL | GTK_SHRINK, GTK_FILL | GTK_EXPAND, 0, 0);
  gtk_widget_show (button);

  arrow = gtk_arrow_new (GTK_ARROW_LEFT, GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (button), arrow);
  gtk_widget_show (arrow);

  gtk_signal_connect (GTK_OBJECT (button), "clicked",
                      GTK_SIGNAL_FUNC (add_selected_to_playlist),
                      clist);

  /* the message label */
  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_IN);
  gtk_box_pack_end (GTK_BOX (main_vbox), frame, FALSE, FALSE, 0);
  gtk_widget_show (frame);
  
  msg_label = gtk_label_new (default_msg);
  gtk_container_add (GTK_CONTAINER (frame), msg_label);
  gtk_widget_show (msg_label);

  log_handler = g_log_set_handler (NULL, G_LOG_LEVEL_MESSAGE, 
                                   (GLogFunc) show_message, msg_label);

  gtk_signal_connect_object (GTK_OBJECT (msg_label), "destroy",
                             g_log_remove_handler,
                             (GtkObject *) log_handler);
}

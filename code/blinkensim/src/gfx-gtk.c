/* blinkensim - a Blinkenlights simulator
 *
 * Copyright (c) 2001-2003  Sven Neumann <sven@gimp.org>
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

#include <blib/blib-gtk.h>

#include <gdk/gdkkeysyms.h>

#include "gfx.h"


static gboolean  gfx_window_state_event       (GtkWidget           *widget,
                                               GdkEventWindowState *event);
static gboolean  gfx_window_key_pressed       (GtkWidget           *widget,
                                               GdkEventKey         *event,
                                               gpointer             data);
static void      gfx_window_toggle_fullscreen (GtkWidget           *window);
static void      gfx_window_unfullscreen      (GtkWidget           *window);


gboolean
gfx_init (gint    *argc,
          gchar ***argv,
          GError **error)
{
  gtk_init (argc, argv);

  return TRUE;
}

void
gfx_close (void)
{
}

GObject *
gfx_view_new (BTheme     *theme,
              GMainLoop  *loop,
              GError    **error)
{
  GtkWidget *window;
  GtkWidget *view;

  view = b_view_gtk_new (theme, error);

  if (!view)
    return FALSE;

  g_signal_connect_swapped (view, "destroy",
                            G_CALLBACK (g_main_loop_quit), loop);

  window = gtk_window_new (GTK_WINDOW_TOPLEVEL);
  gtk_window_set_title (GTK_WINDOW (window),
                        b_object_get_name (B_OBJECT (theme)));

  gtk_widget_add_events (window, GDK_KEY_PRESS_MASK | GDK_STRUCTURE_MASK);

  g_signal_connect (window, "window_state_event",
                    G_CALLBACK (gfx_window_state_event),
                    NULL);
  g_signal_connect (window, "key_press_event",
                    G_CALLBACK (gfx_window_key_pressed),
                    NULL);

  gtk_container_add (GTK_CONTAINER (window), view);
  gtk_widget_show (view);

  gtk_widget_show (window);

  return G_OBJECT (view);
}

void
gfx_view_update (GObject      *view,
                 const guchar *data)
{
  b_view_gtk_update (B_VIEW_GTK (view), data);
}


static gboolean
gfx_window_state_event (GtkWidget           *widget,
                        GdkEventWindowState *event)
{
#if GTK_CHECK_VERSION(2,2,0)
  if (event->changed_mask & GDK_WINDOW_STATE_FULLSCREEN)
    {
      g_object_set_data (G_OBJECT (widget),
                         "fullscreen",
                         GINT_TO_POINTER (event->new_window_state &
                                          GDK_WINDOW_STATE_FULLSCREEN));
    }
#endif

  return FALSE;
}

static gboolean
gfx_window_key_pressed (GtkWidget   *widget,
                        GdkEventKey *event,
                        gpointer     data)
{
  switch (event->keyval)
    {
    case GDK_f:
    case GDK_F:
    case GDK_F11:
      gfx_window_toggle_fullscreen (widget);
      break;

    case GDK_Escape:
#if GTK_CHECK_VERSION(2,2,0)
      if (g_object_get_data (G_OBJECT (widget), "fullscreen"))
        {
          gtk_window_unfullscreen (GTK_WINDOW (widget));
          return TRUE;
        }
#endif
      /*  fallthru  */

    case GDK_q:
    case GDK_Q:
      gtk_widget_destroy (widget);
      break;

    default:
      break;
    }

  return TRUE;
}

static void
gfx_window_toggle_fullscreen (GtkWidget *window)
{
#if GTK_CHECK_VERSION(2,2,0)
  if (g_object_get_data (G_OBJECT (window), "fullscreen"))
    {
      gtk_window_unfullscreen (GTK_WINDOW (window));
    }
  else
    {
      gtk_window_fullscreen (GTK_WINDOW (window));
    }
#else
  g_printerr ("Sorry, fullscreen mode requires GTK+-2.2.\n"
              "This version of blinkensim was compiled against an earlier version.\n");
#endif
}

static void
gfx_window_unfullscreen (GtkWidget *window)
{
#if GTK_CHECK_VERSION(2,2,0)
  if (g_object_get_data (G_OBJECT (window), "fullscreen"))
    {
      gtk_window_unfullscreen (GTK_WINDOW (window));
    }
#endif
}

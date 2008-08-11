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
#include "blmovieview.h"
#include "blpreview.h"


static void     bl_movie_view_class_init  (BlMovieViewClass *class);
static void     bl_movie_view_init        (BlMovieView      *view);
static void     bl_movie_view_destroy     (GtkObject        *object);


static GtkVBoxClass *parent_class = NULL;


GtkType
bl_movie_view_get_type (void)
{
  static GtkType view_type = 0;

  if (!view_type)
    {
      GtkTypeInfo view_info =
      {
	"BlMovieView",
	sizeof (BlMovieView),
	sizeof (BlMovieViewClass),
	(GtkClassInitFunc) bl_movie_view_class_init,
	(GtkObjectInitFunc) bl_movie_view_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL
      };

      view_type = gtk_type_unique (gtk_vbox_get_type (), &view_info);
    }
  
  return view_type;
}

static void
bl_movie_view_class_init (BlMovieViewClass *class)
{
  GtkObjectClass *object_class;

  parent_class = gtk_type_class (gtk_vbox_get_type ());

  object_class = GTK_OBJECT_CLASS (class);

  object_class->destroy = bl_movie_view_destroy;
}

static void
bl_movie_view_init (BlMovieView *view)
{
  view->current    = NULL;
  view->preview    = NULL;
  view->movie      = NULL;
  view->timeout    = 0;
  view->timeout_id = 0;  
}

static void
bl_movie_view_destroy (GtkObject *object)
{
  BlMovieView *view;

  view = BL_MOVIE_VIEW (object);

  if (view->timeout_id)
    {
      g_source_remove (view->timeout_id);
      view->timeout_id = 0;
    }

  if (view->movie)
    {
      gtk_object_unref (GTK_OBJECT (view->movie));
      view->movie = NULL;
    }

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}


static gboolean
bl_movie_view_set_time (BlMovieView *view)
{
  view->timeout_id = 0;
  gtk_adjustment_set_value (view->adjustment, 
                            view->adjustment->value + 
                            (gdouble) view->timeout / 1000);
  
  return FALSE;
}

static void
bl_movie_view_adjustment_value_changed (GtkAdjustment *adjustment,
                                        BlMovieView   *view)
{
  BlMovie *movie;

  if (view->timeout_id)
    {
      g_source_remove (view->timeout_id);
      view->timeout_id = 0;
    }

  movie = view->movie;

  if (movie)
    {
      GList *list;

      list = bl_movie_get_frame_at_time (movie, 
                                         view->current,
                                         1000.0 * adjustment->value);

      if (list != view->current)
        {
          guchar *data;

          view->current = list;

          data = list ? ((BlMovieFrame *) list->data)->data : NULL;
          bl_preview_set_data (view->preview, 
                               movie->width, movie->height, 
                               data);
        }

      if (list &&
          gtk_toggle_button_get_active (GTK_TOGGLE_BUTTON (view->toggle)))
        {
          BlMovieFrame *thiz;
          BlMovieFrame *next;
          
          thiz = (BlMovieFrame *) list->data;

          if (list->next)
            {
              next = (BlMovieFrame *) list->next->data;
              view->timeout = MIN (next->start - thiz->start, 200);
            }
          else
            {
              view->timeout = MIN (movie->duration - thiz->start, 200);
            }

          if (view->timeout > 0)
            view->timeout_id = 
              g_timeout_add (view->timeout,
                             (GSourceFunc) bl_movie_view_set_time,
                             view);
        }
    }
  else
    {
      bl_preview_set_data (view->preview, 0, 0, NULL);
    }
}

GtkWidget *
bl_movie_view_new (BlPreview *preview)
{
  BlMovieView *view;
  GtkWidget   *frame;
  GtkWidget   *vbox;
  GtkWidget   *hbox;
  GtkWidget   *arrow;
  GtkWidget   *label;

  view = BL_MOVIE_VIEW (gtk_object_new (BL_TYPE_MOVIE_VIEW, NULL));

  view->preview = preview;
                
  view->adjustment = 
    GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 0, 0.01, 1, 0.01));
  gtk_signal_connect (GTK_OBJECT (view->adjustment), "value_changed",
                      GTK_SIGNAL_FUNC (bl_movie_view_adjustment_value_changed),
                      view);

  hbox = gtk_hbox_new (FALSE, 4);
  gtk_box_pack_start (GTK_BOX (view), hbox, FALSE, FALSE, 2);
  gtk_widget_show (hbox);

  view->toggle = gtk_toggle_button_new ();
  gtk_toggle_button_set_active (GTK_TOGGLE_BUTTON (view->toggle), FALSE);
  gtk_widget_set_sensitive (view->toggle, FALSE);
  gtk_box_pack_start (GTK_BOX (hbox), view->toggle, FALSE, FALSE, 0);
  gtk_widget_show (view->toggle);

  gtk_signal_connect_object (GTK_OBJECT (view->toggle), "toggled",
                             GTK_SIGNAL_FUNC (gtk_adjustment_value_changed),
                             GTK_OBJECT (view->adjustment));

  arrow = gtk_arrow_new (GTK_ARROW_RIGHT, GTK_SHADOW_IN);
  gtk_container_add (GTK_CONTAINER (view->toggle), arrow);
  gtk_widget_show (arrow);  

  view->slider = gtk_hscale_new (view->adjustment);
  gtk_scale_set_digits (GTK_SCALE (view->slider), 2);
  gtk_widget_set_sensitive (view->slider, FALSE);
  gtk_box_pack_end (GTK_BOX (hbox), view->slider, TRUE, TRUE, 0);
  gtk_widget_show (view->slider);

  frame = gtk_frame_new (NULL);
  gtk_frame_set_shadow_type (GTK_FRAME (frame), GTK_SHADOW_ETCHED_IN);
  gtk_box_pack_start (GTK_BOX (view), frame, TRUE, FALSE, 0);
  gtk_widget_show (frame);

  vbox = gtk_vbox_new (FALSE, 2);
  gtk_container_add (GTK_CONTAINER (frame), vbox);
  gtk_widget_show (vbox);

  view->name = label = gtk_label_new (NULL);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);

  view->description = label = gtk_label_new (NULL);
  gtk_label_set_justify (GTK_LABEL (label), GTK_JUSTIFY_LEFT);
  gtk_label_set_line_wrap (GTK_LABEL (label), TRUE);
  gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
  gtk_widget_show (label);
  
  return GTK_WIDGET (view);
}

void
bl_movie_view_set_movie (BlMovieView *view,
                         BlMovie     *movie)
{
  g_return_if_fail (view != NULL);
  g_return_if_fail (BL_IS_MOVIE_VIEW (view));

  g_return_if_fail (movie == NULL || BL_IS_MOVIE (movie));

  if (view->timeout_id)
    {
      g_source_remove (view->timeout_id);
      view->timeout_id = 0;
    }

  if (view->movie)
    gtk_object_unref (GTK_OBJECT (view->movie));

  if (movie)
    {
      if (bl_movie_load (movie))
        {
          gtk_object_ref (GTK_OBJECT (movie));
          gtk_object_sink (GTK_OBJECT (movie));
        }
      else
        movie = NULL;
    }

  view->movie = movie;
  view->current = NULL;

  gtk_label_set_text (GTK_LABEL (view->name), 
                      movie ? movie->name : NULL);
  gtk_label_set_text (GTK_LABEL (view->description), 
                      movie ? movie->description : NULL);

  view->adjustment->value = 0.0;
  view->adjustment->upper = movie ? (gdouble) movie->duration / 1000.0 : 0.0;
  gtk_adjustment_changed (view->adjustment);
  gtk_adjustment_value_changed (view->adjustment);

  gtk_widget_set_sensitive (view->slider, movie != NULL);
  gtk_widget_set_sensitive (view->toggle, movie != NULL);
}

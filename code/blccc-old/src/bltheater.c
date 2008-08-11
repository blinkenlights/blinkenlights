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

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <gtk/gtk.h>

#include "bltypes.h"
#include "blmovie.h"
#include "blpong.h"
#include "blpreview.h"
#include "bltheater.h"


static void     bl_theater_class_init (BlTheaterClass *class);
static void     bl_theater_init       (BlTheater      *view);
static void     bl_theater_destroy    (GtkObject      *object);
static gboolean bl_theater_next_frame (BlTheater      *theater);


enum 
{
  MOVIE_STARTED,
  MOVIE_FINISHED,
  LAST_SIGNAL
};
static guint bl_theater_signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;


GtkType
bl_theater_get_type (void)
{
  static GtkType theater_type = 0;

  if (!theater_type)
    {
      GtkTypeInfo theater_info =
      {
	"BlTheater",
	sizeof (BlTheater),
	sizeof (BlTheaterClass),
	(GtkClassInitFunc) bl_theater_class_init,
	(GtkObjectInitFunc) bl_theater_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL
      };

      theater_type = gtk_type_unique (gtk_object_get_type (), &theater_info);
    }
  
  return theater_type;
}

static void
bl_theater_class_init (BlTheaterClass *class)
{
  GtkObjectClass *object_class;

  parent_class = gtk_type_class (gtk_object_get_type ());

  object_class = GTK_OBJECT_CLASS (class);

  bl_theater_signals[MOVIE_STARTED] = 
    gtk_signal_new ("movie_started",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (BlTheaterClass, movie_started),
		    gtk_marshal_NONE__POINTER, 
                    GTK_TYPE_NONE, 
                    1, GTK_TYPE_POINTER);
  bl_theater_signals[MOVIE_FINISHED] = 
    gtk_signal_new ("movie_finished",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (BlTheaterClass, movie_finished),
		    gtk_marshal_NONE__POINTER, 
                    GTK_TYPE_NONE, 
                    1, GTK_TYPE_POINTER);
  gtk_object_class_add_signals (object_class, bl_theater_signals, LAST_SIGNAL);

  class->movie_started  = NULL;
  class->movie_finished = NULL;

  object_class->destroy = bl_theater_destroy;
}

static void
bl_theater_init (BlTheater *theater)
{
  theater->width       = 0;
  theater->height      = 0;

  theater->socks       = NULL;
  theater->n_socks     = 0;

  theater->packet      = NULL;
  theater->empty       = NULL;
  theater->frame_count = 0;

  theater->timeout_id  = 0;  

  theater->movie       = NULL;
  theater->current     = NULL;

  theater->preview     = NULL;
}

static void
bl_theater_destroy (GtkObject *object)
{
  BlTheater *theater;

  theater = BL_THEATER (object);

  if (theater->pong)
    {
      gtk_object_unref (GTK_OBJECT (theater->pong));
      theater->pong = NULL;
    }

  if (theater->timeout_id)
    {
      g_source_remove (theater->timeout_id);
      theater->timeout_id = 0;
    }

  if (theater->movie)
    {
      gtk_object_unref (GTK_OBJECT (theater->movie));
      theater->movie = NULL;
    }

  if (theater->packet)
    {
      g_free (theater->packet);
      theater->packet = NULL;
    }
  if (theater->empty)
    {
      g_free (theater->empty);
      theater->empty = NULL;
    }
  
  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
bl_theater_set_frame_data (BlTheater *theater,
                           guchar    *data)
{
  Packet *packet;
  gint    i;

  packet = data ? theater->packet : theater->empty;

  if (theater->n_socks > 0)
    {
      packet->count = theater->frame_count;
      if (data)
        memcpy (&packet->data, data, theater->width * theater->height);
    }
      
  for (i = 0; i < theater->n_socks; i++)
    send (theater->socks[i], 
          packet, sizeof (Packet) + theater->width * theater->height - 1, 0);

  if (theater->preview)
    bl_preview_set_data (theater->preview, 
                         theater->width, theater->height, data);

  theater->frame_count++;
}

static gboolean
bl_theater_movie_finished (BlTheater *theater)
{
  BlMovie *movie;

  theater->timeout_id = 0;

  if (!theater->movie)
    return FALSE;

  movie = theater->movie;
  theater->movie = NULL;

  bl_theater_set_frame_data (theater, NULL);

  gtk_signal_emit (GTK_OBJECT (theater), 
                   bl_theater_signals[MOVIE_FINISHED], movie);

  gtk_object_unref (GTK_OBJECT (movie));

  return FALSE;
}

static gboolean
bl_theater_next_frame (BlTheater *theater)
{
  BlMovie      *movie;
  BlMovieFrame *thiz;
  BlMovieFrame *next;
  gint          timeout = 0;

  theater->timeout_id = 0;

  movie = theater->movie;

  if (!movie || !movie->frames)
    return FALSE;

  if (theater->current)
    theater->current = theater->current->next;
  else
    theater->current = movie->frames;

  if (!theater->current)
    {
      bl_theater_set_frame_data (theater, NULL);      
      return FALSE;
    }

  thiz = (BlMovieFrame *) theater->current->data;

  bl_theater_set_frame_data (theater, thiz->data);
  
  if (theater->current->next)
    {
      next = (BlMovieFrame *) theater->current->next->data;
      timeout = next->start - thiz->start;
      if (timeout > 0)
        theater->timeout_id = 
          g_timeout_add (timeout,
                         (GSourceFunc) bl_theater_next_frame,
                         theater);
    }
  else
    {
      timeout = movie->duration - thiz->start;
      if (timeout > 0)
        theater->timeout_id = 
          g_timeout_add (timeout,
                         (GSourceFunc) bl_theater_movie_finished,
                         theater);
    }

  return FALSE;
}

static void
pong_started (BlPong    *pong,
              BlTheater *theater)
{
  g_print ("%s\n", G_GNUC_FUNCTION);
  if (theater->timeout_id)
    {
      g_source_remove (theater->timeout_id);
      theater->timeout_id = 0;
      bl_theater_set_frame_data (theater, NULL);
    }
}

static void
pong_finished (BlPong    *pong,
               BlTheater *theater)
{
  g_print ("%s\n", G_GNUC_FUNCTION);
  bl_theater_set_frame_data (theater, NULL);
  bl_theater_next_frame (theater);
}

static void
pong_new_frame (BlPong    *pong,
                BlTheater *theater)
{
  bl_theater_set_frame_data (theater, pong->matrix);
}

BlTheater *
bl_theater_new (gint    width,
                gint    height,
                gint   *socks,
                gint    n_socks,
                BlPong *pong)
{
  BlTheater *theater;

  g_return_val_if_fail (width > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);

  g_return_val_if_fail (socks != NULL, NULL);
  g_return_val_if_fail (n_socks > 0, NULL);  

  g_return_val_if_fail (pong == NULL || BL_IS_PONG (pong), NULL);  
  g_return_val_if_fail (pong == NULL || 
                        (pong->width == width && pong->height == height), 
                        NULL);

  theater = BL_THEATER (gtk_object_new (BL_TYPE_THEATER, NULL));

  theater->width  = width;
  theater->height = height;

  theater->packet = g_malloc0 (sizeof (Packet) + width * height);
  theater->packet->magic  = htonl (0xDEADBEEF);
  theater->packet->width  = htons (width);
  theater->packet->height = htons (height);

  theater->empty = g_malloc0 (sizeof (Packet) + width * height);
  theater->empty->magic  = htonl (0xDEADBEEF);
  theater->empty->width  = htons (width);
  theater->empty->height = htons (height);

  theater->socks   = socks;
  theater->n_socks = n_socks;
  
  if (pong)
    {
      gtk_object_ref (GTK_OBJECT (pong));
      gtk_object_sink (GTK_OBJECT (pong));
      theater->pong = pong;

      gtk_signal_connect (GTK_OBJECT (pong), "started",
                          GTK_SIGNAL_FUNC (pong_started),
                          theater);
      gtk_signal_connect (GTK_OBJECT (pong), "finished",
                          GTK_SIGNAL_FUNC (pong_finished),
                          theater);
      gtk_signal_connect (GTK_OBJECT (pong), "new_frame",
                          GTK_SIGNAL_FUNC (pong_new_frame),
                          theater);
    }

  return theater;
}

void
bl_theater_set_preview (BlTheater  *theater,
                        BlPreview  *preview)
{
  g_return_if_fail (theater != NULL);
  g_return_if_fail (BL_IS_THEATER (theater));

  g_return_if_fail (preview != NULL);
  g_return_if_fail (BL_IS_PREVIEW (preview));

  g_return_if_fail (theater->preview == NULL);

  theater->preview = preview;
}

void
bl_theater_set_movie (BlTheater *theater,
                      BlMovie   *movie)
{
  g_return_if_fail (theater != NULL);
  g_return_if_fail (BL_IS_THEATER (theater));

  g_return_if_fail (movie == NULL || BL_IS_MOVIE (movie));
  g_return_if_fail (movie == NULL || 
                    (movie->width == theater->width && 
                     movie->height == theater->height));
  
  if (theater->movie)
    gtk_object_unref (GTK_OBJECT (theater->movie));

  theater->movie = NULL;
  
  if (movie)
    {
      gtk_object_ref (GTK_OBJECT (movie));
      gtk_object_sink (GTK_OBJECT (movie));
    }

  theater->movie = movie;
  theater->current = NULL;

  if (theater->movie && 
      (!theater->pong || !theater->pong->timeout_id))  /* FIXME */
    {
      gtk_signal_emit (GTK_OBJECT (theater), 
                       bl_theater_signals[MOVIE_STARTED], theater->movie);

      bl_theater_next_frame (theater);
    }
}

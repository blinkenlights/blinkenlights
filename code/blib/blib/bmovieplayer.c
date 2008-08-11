/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2002  The Blinkenlights Crew
 +                     Michael Natterer <mitch@gimp.org>
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

#include <glib-object.h>
#include <string.h>

#include "btypes.h"
#include "bmodule.h"
#include "bmodule-utils.h"
#include "bmovie.h"
#include "bmovieplayer.h"
#include "bparams.h"


enum
{
  PROP_0,
  PROP_MOVIE,
  PROP_REVERSE,
  PROP_CLEAR,
  PROP_HALIGN,
  PROP_VALIGN
};

static void     b_movie_player_class_init   (BMoviePlayerClass *klass);
static void     b_movie_player_init         (BMoviePlayer      *player);
static void     b_movie_player_finalize     (GObject           *object);
static void     b_movie_player_set_property (GObject           *object,
                                             guint              property_id,
                                             const GValue      *value,
                                             GParamSpec        *pspec);
static gboolean b_movie_player_query        (gint               width,
                                             gint               height,
                                             gint               channels,
                                             gint               maxval);
static gboolean b_movie_player_prepare      (BModule           *module,
                                             GError           **error);
static void     b_movie_player_relax        (BModule           *module);
static void     b_movie_player_start        (BModule           *module);
static gint     b_movie_player_tick         (BModule           *module);
static void     b_movie_player_describe     (BModule           *module,
                                             const gchar      **title,
                                             const gchar      **description,
                                             const gchar      **author);
static gint     b_movie_player_next_frame   (BMoviePlayer      *player);
static void     b_movie_player_request_stop (BModule           *module);
static void
           b_movie_player_real_request_stop (BModule           *module);


static BModuleClass *parent_class = NULL;


GType
b_movie_player_get_type (void)
{
  static GType player_type = 0;

  if (! player_type)
    {
      static const GTypeInfo player_info =
      {
        sizeof (BMoviePlayerClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_movie_player_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BMoviePlayer),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_movie_player_init,
      };

      player_type = g_type_register_static (B_TYPE_MODULE,
                                            "BMoviePlayer",
                                            &player_info, 0);
    }

  return player_type;
}

static void
b_movie_player_class_init (BMoviePlayerClass *klass)
{
  GObjectClass *object_class;
  GParamSpec   *param_spec;
  BModuleClass *module_class;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->set_property = b_movie_player_set_property;
  object_class->finalize     = b_movie_player_finalize;

  param_spec = b_param_spec_filename ("movie", NULL,
                                      "The filename of the movie to play.",
                                      NULL,
                                      G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_MOVIE, param_spec);

  param_spec = g_param_spec_boolean ("reverse", NULL,
                                     "Allows to play the movie backwards.",
                                     FALSE,
                                     G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_REVERSE, param_spec);

  param_spec = g_param_spec_boolean ("clear", NULL,
                                     "Clear the screen before starting.",
                                     TRUE,
                                     G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_CLEAR, param_spec);

  param_spec = g_param_spec_double ("h-align", NULL,
                                    "Horizontal alignment on the screen.",
                                    0.0, 1.0, 0.5,
                                    G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_HALIGN, param_spec);

  param_spec = g_param_spec_double ("v-align", NULL,
                                    "Vertical alignment on the screen.",
                                    0.0, 1.0, 0.5,
                                    G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_VALIGN, param_spec);

  module_class->query    = b_movie_player_query;
  module_class->prepare  = b_movie_player_prepare;
  module_class->relax    = b_movie_player_relax;
  module_class->start    = b_movie_player_start;
  module_class->tick     = b_movie_player_tick;
  module_class->describe = b_movie_player_describe;

  klass->request_stop    = b_movie_player_real_request_stop;
}

static void
b_movie_player_init (BMoviePlayer *player)
{
  player->current = NULL;
}

static void
b_movie_player_finalize (GObject *object)
{
  BMoviePlayer *player;

  player = B_MOVIE_PLAYER (object);

  if (player->movie)
    {
      g_object_unref (G_OBJECT (player->movie));
      player->movie = NULL;
    }
  if (player->filename)
    {
      g_free (player->filename);
      player->filename = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
b_movie_player_set_property (GObject      *object,
                             guint         property_id,
                             const GValue *value,
                             GParamSpec   *pspec)
{
  BMoviePlayer *player = B_MOVIE_PLAYER (object);

  switch (property_id)
    {
    case PROP_MOVIE:
      if (player->movie)
        {
          g_object_unref (player->movie);
          player->movie   = NULL;
          player->current = NULL;
        }

      if (player->filename)
        g_free (player->filename);

      player->filename = g_value_dup_string (value);
      break;

    case PROP_REVERSE:
      player->reverse = g_value_get_boolean (value);
      break;

    case PROP_CLEAR:
      player->clear = g_value_get_boolean (value);
      break;

    case PROP_HALIGN:
      player->halign = g_value_get_double (value);
      break;

    case PROP_VALIGN:
      player->valign = g_value_get_double (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
b_movie_player_query (gint  width,
                      gint  height,
                      gint  channels,
                      gint  maxval)
{
  return (width > 0 && height > 0 && channels == 1 && maxval == 255);
}

static gboolean
b_movie_player_prepare (BModule  *module,
                        GError  **error)
{
  BMoviePlayer *player = B_MOVIE_PLAYER (module);

  if (!player->filename)
    {
      g_set_error (error, 0, 0, "No movie configured.");
      return FALSE;
    }

  player->current = NULL;

  if (player->movie)
    g_object_unref (player->movie);

  player->movie = b_movie_new_from_file (player->filename, FALSE, error);
  if (!player->movie)
    {
      if (error && *error)
        {
          gchar *tmp = g_strdup_printf ("movie '%s': %s",
                                        player->filename, (*error)->message);

          g_free ((*error)->message);
          (*error)->message = tmp;
        }

      return FALSE;
    }

  if (module->channels != player->movie->channels)
    {
      g_object_unref (player->movie);
      player->movie = NULL;

      g_set_error (error, 0, 0,
                   "Can't handle movie '%s' with more than one channel.",
                   player->filename);
      return FALSE;
    }

  player->xoffset =
    (gdouble)(module->width  - player->movie->width)  * player->halign;
  player->yoffset =
    (gdouble)(module->height - player->movie->height) * player->valign;

  return TRUE;
}

static void
b_movie_player_relax (BModule *module)
{
  BMoviePlayer *player = B_MOVIE_PLAYER (module);

  if (player->movie)
    {
      g_object_unref (player->movie);
      player->movie = NULL;
    }

  player->current = NULL;
}

static void
b_movie_player_start (BModule *module)
{
  BMoviePlayer *player = B_MOVIE_PLAYER (module);
  gint          timeout;

  if (player->clear)
    b_module_fill (module, 0);

  timeout = b_movie_player_next_frame (player);

  if (timeout > 0)
    b_module_ticker_start (module, timeout);
}

static gint
b_movie_player_tick (BModule *module)
{
  return b_movie_player_next_frame (B_MOVIE_PLAYER (module));
}

static void
b_movie_player_describe (BModule      *module,
                         const gchar **title,
                         const gchar **description,
                         const gchar **author)
{
  BMoviePlayer *player = B_MOVIE_PLAYER (module);
  BMovie       *movie  = player->movie;

  if (movie)
    {
      *title       = (movie->title ?
                      movie->title : b_object_get_name (B_OBJECT (movie)));
      *description = movie->description;
      *author      = movie->author;
    }
  else
    {
      B_MODULE_CLASS (parent_class)->describe (module,
                                               title, description, author);
    }
}

static gint
b_movie_player_next_frame (BMoviePlayer *player)
{
  BMovieFrame *frame;
  BModule     *module;

  do
    {
      if (player->current)
        {
          if (player->reverse)
            player->current = g_list_previous (player->current);
          else
            player->current = g_list_next (player->current);
        }
      else if (player->movie)
        {
          if (player->reverse)
            player->current = g_list_last (player->movie->frames);
          else
            player->current = g_list_first (player->movie->frames);
        }

      if (! player->current) /* movie finished */
        {
          b_movie_player_request_stop (B_MODULE (player));
          return 0;
        }

      frame = (BMovieFrame *) player->current->data;
    }
  while (frame->duration <= 0);

  module = B_MODULE (player);

  {
    BMovie  *movie  = B_MOVIE (player->movie);
    guchar  *s, *d  = module->buffer;
    gint     max    = movie->maxval;
    gint     x, y;

    for (y = 0; y < module->height; y++, d += module->width)
      {
        if (y - player->yoffset < 0 || y - player->yoffset >= movie->height)
          continue;

        s = frame->data + (y - player->yoffset) * movie->width;

        for (x = 0; x < module->width; x++)
          {
            if (x - player->xoffset < 0 || x - player->xoffset >= movie->width)
              continue;

            if (max == 255)
              d[x] = s[x - player->xoffset];
            else
              d[x] = ((gint) (s[x - player->xoffset]) * 255) / max;
          }
      }
  }

  b_module_paint (module);

  return frame->duration;
}

static void
b_movie_player_request_stop (BModule *module)
{
  B_MOVIE_PLAYER_GET_CLASS (module)->request_stop (module);
}

static void
b_movie_player_real_request_stop (BModule *module)
{
  b_module_request_stop (module);
}

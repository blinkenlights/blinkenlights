/* blccc - Blinkenlights Chaos Control Center
 *
 * Copyright (c) 2001-2002  Sven Neumann <sven@gimp.org>
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

#include <stdlib.h>

#include <blib/blib.h>

#include "bltypes.h"

#include "blapp.h"
#include "blccc.h"
#include "blconfig.h"
#include "blisdn.h"
#include "blondemand.h"
#include "blplaylist.h"
#include "blplaylistitem.h"
#include "bltheater.h"


static void   bl_ccc_class_init (BlCccClass *class);
static void   bl_ccc_init       (BlCcc      *ccc);
static void   bl_ccc_finalize   (GObject    *object);


static GObjectClass *parent_class = NULL;


GType
bl_ccc_get_type (void)
{
  static GType ccc_type = 0;

  if (!ccc_type)
    {
      static const GTypeInfo ccc_info =
      {
        sizeof (BlCccClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) bl_ccc_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data     */
	sizeof (BlCcc),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) bl_ccc_init,
      };

      ccc_type = g_type_register_static (G_TYPE_OBJECT,
                                         "BlCcc", &ccc_info, 0);
    }

  return ccc_type;
}

static void
bl_ccc_class_init (BlCccClass *class)
{
  GObjectClass *object_class;

  parent_class = g_type_class_peek_parent (class);

  object_class = G_OBJECT_CLASS (class);

  object_class->finalize = bl_ccc_finalize;
}

static void
bl_ccc_init (BlCcc *ccc)
{
  ccc->config      = NULL;
  ccc->mutex       = g_mutex_new ();
  ccc->theater     = NULL;
  ccc->playlist    = NULL;
  ccc->effects     = NULL;
  ccc->isdn        = NULL;
  ccc->active_app  = NULL;

  g_type_class_ref (B_TYPE_MOVIE_PLAYER);
}

static void
bl_ccc_finalize (GObject *object)
{
  BlCcc *ccc = BL_CCC (object);

  if (ccc->mutex)
    {
      g_mutex_free (ccc->mutex);
      ccc->mutex = NULL;
    }
  if (ccc->isdn)
    {
      g_object_unref (ccc->isdn);
      ccc->isdn = NULL;
    }
  if (ccc->theater)
    {
      g_object_unref (ccc->theater);
      ccc->theater = NULL;
    }
  if (ccc->playlist)
    {
      g_object_unref (ccc->playlist);
      ccc->playlist = NULL;
    }
  if (ccc->effects)
    {
      g_object_unref (ccc->effects);
      ccc->effects = NULL;
    }

  g_type_class_unref (g_type_class_peek (B_TYPE_MOVIE_PLAYER));

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
isdn_incoming_call (BlIsdn     *isdn,
                    BlIsdnLine *line,
                    BlCcc      *ccc)
{
  BlApp *active;
  BlApp *app;

  g_printerr ("Incoming call on line %d: %s for %s\n",
              line->channel, line->calling_number, line->called_number);

  active = ccc->active_app;

#if 0
  /* if the line matches this is the call we already handle */
  if (active && active->line == line)
    return;
#endif

  app = bl_config_select_app (ccc->config, line->called_number);

  if (!app)
    {
      g_printerr (" -> no matching number, go away.\n");
      bl_isdn_call_hangup (isdn, line, BL_ISDN_REASON_NO_USER_RESPONSE);
      return;
    }

  if (app->disabled)
    {
      g_printerr (" -> this application is temporarily disabled, go away.\n");
      bl_isdn_call_hangup (isdn, line, BL_ISDN_REASON_NO_USER_RESPONSE);
      return;
    }

  if (!app->public &&
      !bl_config_authorize_caller (ccc->config, line->calling_number))
    {
      g_printerr (" -> I don't know you, go away.\n");
      bl_isdn_call_hangup (isdn, line, BL_ISDN_REASON_NO_USER_RESPONSE);
      return;
    }

  if (active)
    {
      if (app == active)
        {
         if (ccc->theater->item && ccc->theater->item->module)
            {
              BModule *module = ccc->theater->item->module;

              if (module->num_players <
                  B_MODULE_GET_CLASS (module)->max_players)
                {
                  goto accept;
                }
            }
        }

      if (app->priority > active->priority)
        {
          /* FIXME: change apps */
        }

      g_printerr (" -> sorry, we are busy.\n");
      bl_isdn_call_hangup (isdn, line, BL_ISDN_REASON_USER_BUSY);
      return;
    }

 accept:

  line->app = app;
  g_printerr ("Accepting the call on line %d.\n", line->channel);
  bl_isdn_call_accept (isdn, line, app->sound, app->sound_loop);
}

static void
isdn_state_changed (BlIsdn     *isdn,
                    BlIsdnLine *line,
                    BlCcc      *ccc)
{
  BModuleEvent  event;

  g_printerr ("Line %d went %s.\n",
              line->channel, line->offhook ? "offhook" : "onhook");

  if (! line->app)
    return;

  if (line->offhook)
    {
      BlApp *app  = line->app;

      g_mutex_lock (ccc->mutex);

      if (ccc->active_app != app)
        {
          ccc->active_app = app;

          bl_theater_push_item (ccc->theater, BL_PLAYLIST_ITEM (app));
          bl_theater_play (ccc->theater);
        }

      app->lines = g_list_prepend (app->lines, line);

      g_mutex_unlock (ccc->mutex);

      /*  add a player to the module  */
      event.device_id = line->channel - 1;
      event.type      = B_EVENT_TYPE_PLAYER_ENTERED;
      event.key       = 0;

      bl_ccc_event (ccc, &event);
    }
  else
    {
      BlApp    *app  = line->app;
      gboolean  item_pushed;

      /* EEK */
      item_pushed = (ccc->theater->item == BL_PLAYLIST_ITEM (app));

      if (item_pushed)
        {
          /*  remove the player from the module  */
          event.device_id = line->channel - 1;
          event.type      = B_EVENT_TYPE_PLAYER_LEFT;
          event.key       = 0;

          bl_ccc_event (ccc, &event);
        }

      g_mutex_lock (ccc->mutex);

      app->lines = g_list_remove (app->lines, line);
      line->app = NULL;

      if (item_pushed && !app->lines)
        bl_theater_finish (ccc->theater);

      g_mutex_unlock (ccc->mutex);
    }
}

static void
isdn_key_pressed (BlIsdn     *isdn,
                  BlIsdnLine *line,
                  BModuleKey  key,
                  BlCcc      *ccc)
{
  BModuleEvent  event;

  if (! ccc->active_app)
    return;

  /* FIXME: should remap ids */
  event.device_id = line->channel - 1;
  event.type      = B_EVENT_TYPE_KEY;
  event.key       = key;

  bl_ccc_event (ccc, &event);
}

static void
bl_ccc_set_item (BlCcc          *ccc,
                 BlPlaylistItem *item)
{
  bl_theater_set_item (ccc->theater, item);

  bl_theater_play (ccc->theater);
}

static void
theater_item_finished (BlTheater      *theater,
                       BlPlaylistItem *item,
                       gboolean        pushed,
                       BlCcc          *ccc)
{
  if (ccc->active_app)
    {
      if (ccc->isdn && ccc->active_app->lines)
        {
          GList *list;

          for (list = ccc->active_app->lines; list; list = list->next)
            {
              bl_isdn_call_hangup (ccc->isdn,
                                   list->data,
                                   BL_ISDN_REASON_NORMAL_CALL_CLEARING);
            }
        }

      ccc->active_app = NULL;
    }

  if (pushed)
    {
      bl_theater_pop_item (ccc->theater);
      bl_theater_play (ccc->theater);
    }
  else
    {
      if (ccc->playlist)
        {
          item = bl_playlist_load_next_item (ccc->playlist, FALSE);

          if (! item)
            {
              /* reload the playlist manually because calling
               *  bl_ccc_reload() will cause a recursive mutex
               *  deadlock
               */
              BlPlaylist  *new;
              const gchar *filename;

              g_print ("Playlist returned no item, reloading it...\n");

              filename = b_object_get_filename (B_OBJECT (ccc->playlist));

              new = bl_playlist_new_from_file (filename, ccc->config,
                                               bl_theater_paint_callback,
                                               ccc->theater);

              if (new)
                {
                  g_object_unref (ccc->playlist);
                  ccc->playlist = new;

                  item = bl_playlist_load_next_item (ccc->playlist, FALSE);
                }
            }

          if (item)
            bl_ccc_set_item (ccc, item);
        }
    }
}

static void
bl_ccc_setup_applications (BlCcc *ccc)
{
  GList *list;

  for (list = ccc->config->applications; list; list = list->next)
    {
      BModule *module = BL_PLAYLIST_ITEM (list->data)->module;

      module->width          = ccc->config->width;
      module->height         = ccc->config->height;
      module->channels       = 1;
      module->maxval         = 255;
      module->aspect         = ccc->config->aspect;
      module->buffer         = g_new0 (guchar, module->width * module->height);
      module->owns_buffer    = TRUE;
      module->paint_callback = bl_theater_paint_callback;
      module->paint_data     = ccc->theater;
    }
}

BlCcc *
bl_ccc_new (BlConfig *config)
{
  BlCcc     *ccc;
  BlTheater *theater;
  GError    *error = NULL;

  g_return_val_if_fail (BL_IS_CONFIG (config), NULL);

  theater = bl_theater_new (config);
  if (!theater)
    return NULL;

  ccc = BL_CCC (g_object_new (BL_TYPE_CCC, NULL));
  ccc->config  = g_object_ref (config);
  ccc->theater = theater;

  bl_theater_set_frame_data (theater, NULL);

  g_signal_connect (G_OBJECT (theater), "item_finished",
                    G_CALLBACK (theater_item_finished),
                    ccc);

  if (config->isdn_host)
    {
      ccc->isdn = bl_isdn_new (config, &error);
      if (!ccc->isdn)
        {
          g_printerr ("Could not setup connection to ISDN system "
                      "on %s port %d:\n   %s\n",
                      config->isdn_host, config->isdn_port,
                      error->message);
          g_object_unref (ccc);
          return NULL;
        }
    }

  if (ccc->isdn)
    {
      bl_ccc_setup_applications (ccc);

      g_signal_connect (G_OBJECT (ccc->isdn), "incoming_call",
                        G_CALLBACK (isdn_incoming_call),
                        ccc);
      g_signal_connect (G_OBJECT (ccc->isdn), "state_changed",
                        G_CALLBACK (isdn_state_changed),
                        ccc);
      g_signal_connect (G_OBJECT (ccc->isdn), "key_pressed",
                        G_CALLBACK (isdn_key_pressed),
                        ccc);
    }

  bl_ccc_load (ccc, config->playlist, FALSE);

  ccc->effects = b_effects_new ();

  bl_theater_set_effects (ccc->theater, ccc->effects);

  return ccc;
}

gchar *
bl_ccc_status (BlCcc *ccc)
{
  GString *status;

  g_return_val_if_fail (BL_IS_CCC (ccc), NULL);

  status = g_string_new (NULL);

  g_mutex_lock (ccc->mutex);

  if (ccc->playlist)
    {
      g_string_append_printf (status, "Playlist is '%s' with %d items.\n",
                              b_object_get_name (B_OBJECT (ccc->playlist)),
                              bl_playlist_length (ccc->playlist));
    }
  else
    {
      g_string_append (status, "No playlist loaded.\n");
    }

  if (ccc->effects)
    {
      g_string_append_printf (status, "Current effects: "
                              "invert=%s vflip=%s hflip=%s lmirror=%s rmirror=%s.\n",
                              ccc->effects->invert  ? "ON" : "OFF",
                              ccc->effects->vflip   ? "ON" : "OFF",
                              ccc->effects->hflip   ? "ON" : "OFF",
                              ccc->effects->lmirror ? "ON" : "OFF",
                              ccc->effects->rmirror ? "ON" : "OFF");
    }

  if (ccc->theater->item)
    {
      gchar *str;

      str = bl_playlist_item_describe (ccc->theater->item);
      g_string_append_printf (status, "Current item is a %s.\n", str);
      g_free (str);

      if (bl_theater_is_playing (ccc->theater))
        g_string_append_printf (status, "The movie is playing%s.\n",
                                ccc->active_app ? " on demand" : "");
      else
        g_string_append (status, "The movie is paused.\n");
    }
  else
    {
      g_string_append (status, "No movie.\n");
    }

  {
    GType *module_types;
    guint  n_module_types;
    guint  i;

    g_string_append (status, "Modules: ");

    module_types = g_type_children (B_TYPE_MODULE, &n_module_types);

    for (i = 0; i < n_module_types; i++)
      {
        if (i > 0)
          g_string_append (status, ", ");

        g_string_append (status, g_type_name (module_types[i]));
      }

    g_free (module_types);

    g_string_append (status, "\n");
  }

  {
    GList *recipients;

    recipients = b_sender_list_recipients (ccc->theater->sender);

    if (recipients)
      {
        GList *list;

        g_string_append (status, "Recipients: ");

        for (list = recipients; list; list = g_list_next (list))
          {
            if (list->prev)
              g_string_append (status, ", ");

            g_string_append (status, (gchar *) list->data);
          }

        g_string_append (status, ".\n");

        g_list_foreach (recipients, (GFunc) g_free, NULL);
        g_list_free (recipients);
      }
    else
      {
        g_string_append (status, "No recipients.\n");
      }
  }

  if (ccc->isdn)
    {
      gint i;

      g_string_append_printf (status, "Control through isdn is %s.\n",
                              ccc->isdn->blocked ? "blocked" : "allowed");

      g_string_append_printf (status, "The %d isdn lines are:\n",
                              ccc->isdn->num_lines);

      for (i = 0; i < ccc->isdn->num_lines; i++)
        {
          if (ccc->isdn->lines[i].offhook)
            g_string_append_printf (status, "  %d: offhook (%s for %s)\n", i,
                                    ccc->isdn->lines[i].calling_number,
                                    ccc->isdn->lines[i].called_number);
          else
            g_string_append_printf (status, "  %d: onhook\n", i);
        }
    }

  g_mutex_unlock (ccc->mutex);

  return g_string_free (status, FALSE);
}

gboolean
bl_ccc_reload (BlCcc *ccc)
{
  gchar    *filename = NULL;
  gboolean  retval = FALSE;

  g_return_val_if_fail (BL_IS_CCC (ccc), FALSE);

  g_mutex_lock (ccc->mutex);

  if (ccc->playlist)
    filename = g_strdup (b_object_get_filename (B_OBJECT (ccc->playlist)));

  g_mutex_unlock (ccc->mutex);

  retval = bl_ccc_load (ccc, filename, FALSE);
  g_free (filename);

  return retval;
}

gboolean
bl_ccc_load (BlCcc       *ccc,
             const gchar *filename,
             gboolean     instant_change)
{
  BlPlaylist *new;

  g_return_val_if_fail (BL_IS_CCC (ccc), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);

  g_mutex_lock (ccc->mutex);

  new = bl_playlist_new_from_file (filename, ccc->config,
                                   bl_theater_paint_callback, ccc->theater);

  if (new)
    {
      if (ccc->playlist)
        g_object_unref (ccc->playlist);
      ccc->playlist = new;

      if (!ccc->theater->item)
        {
          BlPlaylistItem *item;

          item = bl_playlist_load_next_item (ccc->playlist, FALSE);

          if (item)
            bl_ccc_set_item (ccc, item);
        }
      else if (instant_change)
        {
          bl_theater_finish (ccc->theater);
        }
    }

  g_mutex_unlock (ccc->mutex);

  return (new != NULL);
}

gchar *
bl_ccc_list (BlCcc *ccc)
{
  gchar *retval = NULL;

  g_return_val_if_fail (BL_IS_CCC (ccc), NULL);

  g_mutex_lock (ccc->mutex);

  if (ccc->playlist)
    retval = bl_playlist_item_describe (BL_PLAYLIST_ITEM (ccc->playlist));

  g_mutex_unlock (ccc->mutex);

  return retval;
}

gchar *
bl_ccc_next (BlCcc *ccc)
{
  gchar *retval = NULL;

  g_return_val_if_fail (BL_IS_CCC (ccc), NULL);

  g_mutex_lock (ccc->mutex);

  bl_theater_finish (ccc->theater);

  if (ccc->theater->item)
    retval = bl_playlist_item_describe (ccc->theater->item);

  g_mutex_unlock (ccc->mutex);

  return retval;
}

void
bl_ccc_kill (BlCcc *ccc)
{
  g_return_if_fail (BL_IS_CCC (ccc));

  g_mutex_lock (ccc->mutex);

  if (ccc->theater)
    bl_theater_kill (ccc->theater);

  g_mutex_unlock (ccc->mutex);
}

void
bl_ccc_event (BlCcc        *ccc,
              BModuleEvent *event)
{
  g_return_if_fail (BL_IS_CCC (ccc));
  g_return_if_fail (event != NULL);

  g_mutex_lock (ccc->mutex);

  bl_theater_event (ccc->theater, event);

  g_mutex_unlock (ccc->mutex);
}

gboolean
bl_ccc_add (BlCcc        *ccc,
            const gchar  *host,
            GError      **error)
{
  gboolean success;

  g_return_val_if_fail (BL_IS_CCC (ccc), FALSE);
  g_return_val_if_fail (host != NULL, FALSE);

  g_mutex_lock (ccc->mutex);

  success = bl_theater_add_recipient (ccc->theater, host, error);

  g_mutex_unlock (ccc->mutex);

  return success;
}

gboolean
bl_ccc_remove (BlCcc        *ccc,
               const gchar  *host,
               GError      **error)
{
  gboolean success;

  g_return_val_if_fail (BL_IS_CCC (ccc), FALSE);
  g_return_val_if_fail (host != NULL, FALSE);

  g_mutex_lock (ccc->mutex);

  success = bl_theater_remove_recipient (ccc->theater, host, error);

  g_mutex_unlock (ccc->mutex);

  return success;
}

gboolean
bl_ccc_app_enable (BlCcc        *ccc,
                   const gchar  *number)
{
  BlApp *app;

  g_return_val_if_fail (BL_IS_CCC (ccc), FALSE);
  g_return_val_if_fail (number != NULL, FALSE);

  app = bl_config_select_app (ccc->config, number);
  if (!app)
    return FALSE;

  if (!app->disabled)
    return TRUE;

  app->disabled = FALSE;
  g_printerr ("Enabled application %s (%s)\n", app->name, app->number);

  return TRUE;
}

gboolean
bl_ccc_app_disable (BlCcc        *ccc,
                    const gchar  *number)
{
  BlApp *app;

  g_return_val_if_fail (BL_IS_CCC (ccc), FALSE);
  g_return_val_if_fail (number != NULL, FALSE);

  app = bl_config_select_app (ccc->config, number);
  if (!app)
    return FALSE;

  if (app->disabled)
    return TRUE;

  app->disabled = TRUE;
  g_printerr ("Disabled application %s (%s)\n", app->name, app->number);

  return TRUE;
}

void
bl_ccc_isdn_block (BlCcc *ccc)
{
  g_return_if_fail (BL_IS_CCC (ccc));

  if (ccc->isdn)
    bl_isdn_block (ccc->isdn);

  bl_theater_finish (ccc->theater);
}

void
bl_ccc_isdn_unblock (BlCcc *ccc)
{
  g_return_if_fail (BL_IS_CCC (ccc));

  if (ccc->isdn)
    bl_isdn_unblock (ccc->isdn);
}

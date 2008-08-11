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

#include <string.h>

#include <blib/blib.h>

#include "bltypes.h"

#include "blmarshal.h"
#include "blconfig.h"
#include "bllogger.h"
#include "blplaylist.h"
#include "blplaylistitem.h"
#include "bltheater.h"


enum
{
  ITEM_FINISHED,
  LAST_SIGNAL
};

static void     bl_theater_class_init              (BlTheaterClass *klass);
static void     bl_theater_init                    (BlTheater      *view);
static void     bl_theater_finalize                (GObject        *object);

static void     bl_theater_set_item_internal       (BlTheater      *theater,
                                                    BlPlaylistItem *item);
static void     bl_theater_set_frame_data_internal (BlTheater      *theater,
                                                    const guchar   *data);
static void     bl_theater_item_finished           (BlTheater      *theater,
                                                    gboolean        relax);
static void     bl_theater_stop_callback           (BModule        *module,
                                                    BlTheater      *theater);


static guint bl_theater_signals[LAST_SIGNAL] = { 0 };

static GObjectClass *parent_class = NULL;


GType
bl_theater_get_type (void)
{
  static GType theater_type = 0;

  if (!theater_type)
    {
      static const GTypeInfo theater_info =
      {
        sizeof (BlTheaterClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) bl_theater_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data     */
	sizeof (BlTheater),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) bl_theater_init,
      };

      theater_type = g_type_register_static (G_TYPE_OBJECT,
                                             "BlTheater",
                                             &theater_info, 0);
    }

  return theater_type;
}

static void
bl_theater_class_init (BlTheaterClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  bl_theater_signals[ITEM_FINISHED] =
    g_signal_new ("item_finished",
		  G_TYPE_FROM_CLASS (klass),
		  G_SIGNAL_RUN_FIRST,
		  G_STRUCT_OFFSET (BlTheaterClass, item_finished),
		  NULL, NULL,
		  bl_marshal_VOID__POINTER_BOOLEAN,
		  G_TYPE_NONE, 2, G_TYPE_POINTER, G_TYPE_BOOLEAN);

  object_class->finalize = bl_theater_finalize;

  klass->item_finished = NULL;
}

static void
bl_theater_init (BlTheater *theater)
{
  theater->width          = 0;
  theater->height         = 0;

  theater->stop_signal_id = 0;

  theater->item           = NULL;
  theater->item_stack     = NULL;

  theater->effects        = NULL;
  theater->sender         = b_sender_new ();
  theater->item_buffer    = NULL;
  theater->frame_buffer   = NULL;
}

static void
bl_theater_finalize (GObject *object)
{
  BlTheater *theater;

  theater = BL_THEATER (object);

  bl_theater_pause (theater);

  if (theater->item)
    g_object_unref (theater->item);

  if (theater->item_stack)
    {
      g_list_foreach (theater->item_stack, (GFunc) g_object_unref, NULL);
      g_list_free (theater->item_stack);
    }

  if (theater->effects)
    g_object_unref (theater->effects);

  if (theater->sender)
    g_object_unref (theater->sender);

  if (theater->item_buffer)
    g_free (theater->item_buffer);

  if (theater->frame_buffer)
    g_free (theater->frame_buffer);

  if (theater->logger)
    {
      bl_logger_stop (theater->logger);
      g_object_unref (theater->logger);
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

void
bl_theater_play (BlTheater *theater)
{
  g_return_if_fail (BL_IS_THEATER (theater));

  if (! theater->stop_signal_id && theater->item)
    {
      theater->stop_signal_id =
        g_signal_connect (G_OBJECT (theater->item->module), "stop",
                          G_CALLBACK (bl_theater_stop_callback),
                          theater);

      b_module_start (theater->item->module);

      if (theater->logger)
        bl_logger_start_module (theater->logger, theater->item->module);

      {
        gchar *desc;

        desc = bl_playlist_item_describe (theater->item);
        g_print ("started item: %s\n", desc);
        g_free (desc);
      }
    }
}

void
bl_theater_pause (BlTheater *theater)
{
  g_return_if_fail (BL_IS_THEATER (theater));

  if (theater->stop_signal_id && theater->item)
    {
      g_signal_handler_disconnect (G_OBJECT (theater->item->module),
                                   theater->stop_signal_id);
      theater->stop_signal_id = 0;

      b_module_stop (theater->item->module);
    }
}

void
bl_theater_finish (BlTheater *theater)
{
  g_return_if_fail (BL_IS_THEATER (theater));

  if (theater->item)
    {
      bl_theater_pause (theater);
      bl_theater_item_finished (theater, TRUE);
    }
}

void
bl_theater_kill (BlTheater *theater)
{
  g_return_if_fail (BL_IS_THEATER (theater));

  bl_theater_pause (theater);
  bl_theater_set_frame_data (theater, NULL);

  if (theater->logger)
    bl_logger_stop (theater->logger);
}

gboolean
bl_theater_is_playing (BlTheater *theater)
{
  g_return_val_if_fail (BL_IS_THEATER (theater), FALSE);

  return (theater->stop_signal_id > 0);
}

void
bl_theater_event (BlTheater    *theater,
                  BModuleEvent *event)
{
  g_return_if_fail (BL_IS_THEATER (theater));
  g_return_if_fail (event != NULL);

  if (theater->item)
    b_module_event (theater->item->module, event);
}

void
bl_theater_set_frame_data (BlTheater    *theater,
                           const guchar *data)
{
  g_return_if_fail (BL_IS_THEATER (theater));

  if (theater->stop_signal_id)
    g_warning ("Someone tried to set frame data while a movie is running.");
  else
    bl_theater_set_frame_data_internal (theater, data);
}

BlTheater *
bl_theater_new (BlConfig *config)
{
  BlTheater *theater;
  GList     *list;
  GError    *error = NULL;
  gint       added_recipients = 0;

  g_return_val_if_fail (BL_IS_CONFIG (config), NULL);

  theater = BL_THEATER (g_object_new (BL_TYPE_THEATER, NULL));

  theater->width  = config->width;
  theater->height = config->height;

  if (! b_sender_configure (theater->sender,
                            config->width, config->height,
                            config->channels, config->maxval))
    {
      g_object_unref (G_OBJECT (theater));
      return NULL;
    }

  for (list = config->recipients; list; list = list->next)
    {
      if (! bl_theater_add_recipient (theater, list->data, &error))
        {
          g_printerr ("Failed to add recipient: %s\n", error->message);
          g_clear_error (&error);
        }
      else
        {
          added_recipients++;
        }
    }

  if (added_recipients == 0)
    g_printerr ("No initial recipients, starting anyway ...\n");
  else
    g_printerr ("Sending to %d recipients ...\n", added_recipients);

  theater->item_buffer  = g_new0 (guchar, theater->width * theater->height);
  theater->frame_buffer = g_new0 (guchar, theater->width * theater->height);

  if (config->logfile)
    theater->logger = bl_logger_new_from_file (config->logfile, NULL);

  return theater;
}

gboolean
bl_theater_add_recipient (BlTheater    *theater,
                          const gchar  *hostname,
                          GError      **error)
{
  gchar    *host;
  gchar    *colon;
  gint      port = MCU_LISTENER_PORT;
  gboolean  retval;

  g_return_val_if_fail (BL_IS_THEATER (theater), FALSE);
  g_return_val_if_fail (hostname != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  host = g_strdup (hostname);

  if ((colon = strrchr (host, ':')))
    {
      b_parse_int (colon + 1, &port);
      *colon = '\0';
    }

  retval = b_sender_add_recipient (theater->sender, -1, host, port, error);

  g_free (host);

  return retval;
}

gboolean
bl_theater_remove_recipient (BlTheater    *theater,
                             const gchar  *hostname,
                             GError      **error)
{
  gchar    *host;
  gchar    *colon;
  gint      port = MCU_LISTENER_PORT;
  gboolean  retval;

  g_return_val_if_fail (BL_IS_THEATER (theater), FALSE);
  g_return_val_if_fail (hostname != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  host = g_strdup (hostname);

  if ((colon = strrchr (host, ':')))
    {
      b_parse_int (colon + 1, &port);
      *colon = '\0';
    }

  retval = b_sender_remove_recipient (theater->sender, host, port, error);

  g_free (host);

  return retval;
}

void
bl_theater_set_effects (BlTheater *theater,
                        BEffects  *effects)
{
  g_return_if_fail (BL_IS_THEATER (theater));
  g_return_if_fail (! effects || B_IS_EFFECTS (effects));

  if (theater->effects)
    g_object_unref (theater->effects);

  theater->effects = effects;

  if (theater->effects)
    g_object_ref (theater->effects);
}

void
bl_theater_set_item (BlTheater      *theater,
                     BlPlaylistItem *item)
{
  g_return_if_fail (BL_IS_THEATER (theater));

  if (item)
    g_return_if_fail (B_IS_MODULE (item->module)             &&
                      item->module->width  == theater->width &&
                      item->module->height == theater->height);

  if (theater->item)
    {
      bl_theater_pause (theater);
      bl_theater_item_finished (theater, TRUE);
    }

  if (item)
    bl_theater_set_item_internal (theater, item);
}

void
bl_theater_push_item (BlTheater      *theater,
                      BlPlaylistItem *item)
{
  g_return_if_fail (BL_IS_THEATER (theater));
  g_return_if_fail (BL_IS_PLAYLIST_ITEM (item));
  g_return_if_fail (B_IS_MODULE (item->module)             &&
                    item->module->width  == theater->width &&
                    item->module->height == theater->height);

  bl_theater_pause (theater);

  if (theater->item)
    theater->item_stack = g_list_prepend (theater->item_stack,
                                          theater->item);

  bl_theater_set_item_internal (theater, item);
}

void
bl_theater_pop_item (BlTheater *theater)
{
  g_return_if_fail (BL_IS_THEATER (theater));

  if (theater->item_stack)
    {
      bl_theater_pause (theater);

      if (theater->item)
        g_object_unref (G_OBJECT (theater->item));

      theater->item = theater->item_stack->data;

      theater->item_stack = g_list_remove (theater->item_stack,
                                           theater->item);
    }
}

gboolean
bl_theater_paint_callback (BModule  *module,
                           guchar   *buffer,
                           gpointer  data)
{
  BlTheater *theater;

  g_return_val_if_fail (B_IS_MODULE (module), FALSE);
  g_return_val_if_fail (BL_IS_THEATER (data), FALSE);

  theater = BL_THEATER (data);

  if (theater->item && theater->item->effects)
    {
      memcpy (theater->item_buffer, buffer, theater->width * theater->height);

      b_effects_apply (theater->item->effects,
                       theater->item_buffer,
                       theater->width, theater->height, 1, 255);

      buffer = theater->item_buffer;
    }

  bl_theater_set_frame_data_internal (BL_THEATER (data), buffer);

  return TRUE;
}


/*  private functions  */

static void
bl_theater_set_item_internal (BlTheater      *theater,
                              BlPlaylistItem *item)
{
  GError *error = NULL;

  g_return_if_fail (BL_IS_THEATER (theater));
  g_return_if_fail (BL_IS_PLAYLIST_ITEM (item));
  g_return_if_fail (B_IS_MODULE (item->module)             &&
                    item->module->width  == theater->width &&
                    item->module->height == theater->height);

  theater->item = g_object_ref (G_OBJECT (item));

  if (!b_module_prepare (item->module, &error))
    {
      gchar *title;

      if (!error)
	{
	  g_warning ("b_module_prepare() didn't return an error!");
	}
      else
	{
	  b_module_describe (item->module, &title, NULL, NULL);
	  g_printerr ("Couldn't prepare module '%s': %s\n",
                      title, error->message);
	  g_free (title);
	}

      bl_theater_item_finished (theater, FALSE);
      return;
    }
}

static void
bl_theater_set_frame_data_internal (BlTheater    *theater,
                                    const guchar *data)
{
  if (data && theater->effects)
    {
      memcpy (theater->frame_buffer, data, theater->width * theater->height);

      b_effects_apply (theater->effects,
                       theater->frame_buffer,
                       theater->width, theater->height, 1, 255);

      data = (const gchar *) theater->frame_buffer;
    }

  b_sender_send_frame (theater->sender, data);
}

static void
bl_theater_item_finished (BlTheater *theater,
                          gboolean   relax)
{
  BlPlaylistItem *item;
  gboolean        pushed;

  if (! theater->item)
    return;

  item = theater->item;

  if (relax)
    b_module_relax (item->module);

  theater->item = NULL;

  pushed = (theater->item_stack != NULL);

  g_signal_emit (G_OBJECT (theater),
                 bl_theater_signals[ITEM_FINISHED], 0,
                 item, pushed);

  g_object_unref (G_OBJECT (item));
}

static void
bl_theater_stop_callback (BModule   *module,
                          BlTheater *theater)
{
  if (theater->stop_signal_id)
    {
      g_signal_handler_disconnect (G_OBJECT (theater->item->module),
                                   theater->stop_signal_id);
      theater->stop_signal_id = 0;
    }

  bl_theater_item_finished (theater, TRUE);
}

/* blccc - Blinkenlights Chaos Control Center
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

#include <stdio.h>
#include <stdlib.h>

#include <blib/blib.h>

#include "bltypes.h"
#include "blconfig.h"
#include "blplaylist.h"
#include "blplaylist-parse.h"


enum
{
  PROP_0,
  PROP_SHUFFLE,
};

static void    bl_playlist_class_init    (BlPlaylistClass *klass);
static void    bl_playlist_init          (BlPlaylist      *playlist);
static void    bl_playlist_finalize      (GObject         *object);
static void    bl_playlist_set_property  (GObject         *object,
                                          guint            property_id,
                                          const GValue    *value,
                                          GParamSpec      *pspec);

static void    bl_playlist_shuffle       (BlPlaylist      *playlist);
static void    bl_playlist_normalize     (BlPlaylist      *playlist);

static gchar * bl_playlist_describe      (BlPlaylistItem  *item);
static void    bl_playlist_apply_effects (BlPlaylistItem  *item,
                                          BEffects        *effects,
                                          gint             loop,
                                          gboolean         reverse,
                                          gdouble          speed);


static BlPlaylistItemClass *parent_class = NULL;


GType
bl_playlist_get_type (void)
{
  static GType playlist_type = 0;

  if (!playlist_type)
    {
      static const GTypeInfo playlist_info =
      {
        sizeof (BlPlaylistClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) bl_playlist_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BlPlaylist),
        0,              /* n_preallocs */
        (GInstanceInitFunc) bl_playlist_init,
      };

      playlist_type = g_type_register_static (BL_TYPE_PLAYLIST_ITEM,
                                              "BlPlaylist",
                                              &playlist_info, 0);
    }

  return playlist_type;
}

static void
bl_playlist_class_init (BlPlaylistClass *klass)
{
  GObjectClass        *object_class;
  GParamSpec          *param_spec;
  BlPlaylistItemClass *item_class;

  object_class = G_OBJECT_CLASS (klass);
  item_class   = BL_PLAYLIST_ITEM_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->set_property = bl_playlist_set_property;
  object_class->finalize     = bl_playlist_finalize;

  param_spec = g_param_spec_boolean ("shuffle", NULL,
                                     "Shuffle the play order.",
                                     FALSE,
                                     G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_SHUFFLE, param_spec);


  item_class->describe      = bl_playlist_describe;
  item_class->apply_effects = bl_playlist_apply_effects;
}

static void
bl_playlist_init (BlPlaylist *playlist)
{
  playlist->config   = NULL;
  playlist->shuffle  = FALSE;
  playlist->buffer   = NULL;
  playlist->items    = NULL;
  playlist->current  = NULL;
}

static void
bl_playlist_finalize (GObject *object)
{
  BlPlaylist *playlist = BL_PLAYLIST (object);

  g_list_foreach (playlist->items, (GFunc) g_object_unref, NULL);
  g_list_free (playlist->items);

  playlist->items   = NULL;
  playlist->current = NULL;

  if (playlist->config)
    {
      g_object_unref (playlist->config);
      playlist->config = NULL;
    }
  if (playlist->buffer)
    {
      g_free (playlist->buffer);
      playlist->buffer = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
bl_playlist_set_property (GObject      *object,
                          guint         property_id,
                          const GValue *value,
                          GParamSpec   *pspec)
{
  BlPlaylist *playlist = BL_PLAYLIST (object);

  switch (property_id)
    {
    case PROP_SHUFFLE:
      playlist->shuffle = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


BlPlaylist *
bl_playlist_new (BlConfig *config)
{
  BlPlaylist *playlist;

  g_return_val_if_fail (BL_IS_CONFIG (config), NULL);

  playlist = g_object_new (BL_TYPE_PLAYLIST, NULL);

  playlist->config = g_object_ref (config);
  playlist->buffer = g_new0 (guchar, config->width * config->height);

  return playlist;
}

BlPlaylist *
bl_playlist_new_from_file (const gchar          *filename,
                           BlConfig             *config,
                           BModulePaintCallback  paint_callback,
                           gpointer              paint_data)
{
  BlPlaylist *playlist;
  GError     *error = NULL;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (BL_IS_CONFIG (config), NULL);
  g_return_val_if_fail (paint_callback != NULL, NULL);

  if (! g_file_test (filename, G_FILE_TEST_IS_REGULAR))
    {
      g_printerr ("Playlist '%s' does not exist or is not a regular file\n",
                  filename);
      return NULL;
    }

  playlist = bl_playlist_new (config);

  b_object_set_filename (B_OBJECT (playlist), filename);

  if (! bl_playlist_parse (playlist, paint_callback, paint_data, &error))
    {
      g_printerr ("Failed to parse playlist '%s': %s\n",
                  b_object_get_name (B_OBJECT (playlist)), error->message);
      g_clear_error (&error);
    }
  else
    {
      bl_playlist_normalize (playlist);

      g_printerr ("Successfully loaded playlist '%s' with %d items\n",
                  b_object_get_name (B_OBJECT (playlist)),
                  bl_playlist_length (playlist));
    }

  return playlist;
}

gint
bl_playlist_length (BlPlaylist *playlist)
{
  g_return_val_if_fail (BL_IS_PLAYLIST (playlist), 0);

  return g_list_length (playlist->items);
}

static BlPlaylistItem *
bl_playlist_load_item_internal (BlPlaylistItem *item)
{
  if (BL_IS_PLAYLIST (item))
    return bl_playlist_load_next_item (BL_PLAYLIST (item), FALSE);

  return item;
}

static BlPlaylistItem *
bl_playlist_load_next (BlPlaylist *playlist,
                       gboolean    wrap_around)
{
  BlPlaylistItem *item;
  BlPlaylistItem *return_item;
  GList          *list;

  if (playlist->current)
    {
      item = (BlPlaylistItem *) playlist->current->data;

      if (BL_IS_PLAYLIST (item))
        {
          return_item = bl_playlist_load_item_internal (item);

          if (return_item)
            return return_item;
        }

      item->count++;

      if (item->loop > item->count)
        {
          return_item = bl_playlist_load_item_internal (item);

          if (return_item)
            return return_item;
        }

      item->count = 0;

      list = g_list_next (playlist->current);
    }
  else
    {
      if (playlist->shuffle)
        bl_playlist_shuffle (playlist);

      list = playlist->items;
    }

  for (; list; list = g_list_next (list))
    {
      item = (BlPlaylistItem *) list->data;

      return_item = bl_playlist_load_item_internal (item);

      if (return_item)
        {
          playlist->current = list;
          return return_item;
        }
    }

  if (wrap_around)
    {
      if (playlist->shuffle)
        bl_playlist_shuffle (playlist);

      /*  start from the beginning  */

      for (list = playlist->items; list; list = g_list_next (list))
        {
          item = (BlPlaylistItem *) list->data;

          return_item = bl_playlist_load_item_internal (item);

          if (return_item)
            {
              playlist->current = list;
              return return_item;
            }
        }
    }

  playlist->current = NULL;

  return NULL;
}

BlPlaylistItem *
bl_playlist_load_next_item (BlPlaylist *playlist,
                            gboolean    wrap_around)
{
  BlPlaylistItem *item;

  g_return_val_if_fail (BL_IS_PLAYLIST (playlist), NULL);

  while ((item = bl_playlist_load_next (playlist, wrap_around)) != NULL)
    {
      if (item->count == 0)
        {
          gdouble limit = (CLAMP (item->likely, 0.001, 1.0) *
                           (gdouble) RAND_MAX);

          if (rand () < limit)
            return item;
          else
            item->count = item->loop;
        }
      else
        {
          return item;
        }
    }

  return NULL;
}

static gint
bl_playlist_sort_random (void)
{
  return (rand () % 2) ? -1 : 1;
}

static void
bl_playlist_shuffle (BlPlaylist *playlist)
{
  playlist->items = g_list_sort (playlist->items,
                                 (GCompareFunc) bl_playlist_sort_random);
}

static void
bl_playlist_normalize (BlPlaylist *playlist)
{
  GList   *list;
  gdouble  max = 0.0;

  for (list = playlist->items; list; list = list->next)
    {
      BlPlaylistItem *item = list->data;

      max = MAX (max, item->likely);
    }

  if (max <= 1.0)
    return;

  for (list = playlist->items; list; list = list->next)
    {
      BlPlaylistItem *item = list->data;

      item->likely = item->likely / max;
    }
}

static gchar *
bl_playlist_describe (BlPlaylistItem *item)
{
  BlPlaylist     *playlist;
  BlPlaylistItem *child;
  GList          *list;
  GString        *str;
  gchar          *tmp;

  playlist = BL_PLAYLIST (item);

  str = g_string_new (NULL);
  g_string_printf (str, "Playlist '%s':\n",
                   b_object_get_name (B_OBJECT (playlist)));

  for (list = playlist->items; list; list = g_list_next (list))
    {
      child = (BlPlaylistItem *) list->data;

      tmp = bl_playlist_item_describe (child);
      g_string_append_printf (str, " %s\n", tmp);
      g_free (tmp);
    }

  return g_string_free (str, FALSE);
}

static void
bl_playlist_apply_effects (BlPlaylistItem *item,
                           BEffects       *effects,
                           gint            loop,
                           gboolean        reverse,
                           gdouble         speed)
{
  BlPlaylist     *playlist;
  BlPlaylistItem *child;
  GList          *list;

  playlist = BL_PLAYLIST (item);

  BL_PLAYLIST_ITEM_CLASS (parent_class)->apply_effects (item, effects,
                                                        loop, reverse, speed);

  for (list = playlist->items; list; list = g_list_next (list))
    {
      child = (BlPlaylistItem *) list->data;

      bl_playlist_item_apply_effects (child, effects, 0, reverse, speed);
    }

  if (reverse)
    playlist->items = g_list_reverse (playlist->items);
}

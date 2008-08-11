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

#include "blconfig.h"
#include "blplaylist.h"
#include "blplaylist-parse.h"
#include "blplaylistitem.h"


typedef enum
{
  PARSER_IN_PLAYLIST = B_PARSER_STATE_USER,
  PARSER_IN_LIST,
  PARSER_IN_ITEM,
  PARSER_IN_PARAM,
  PARSER_FINISH
} ParserState;

typedef struct _ParserData ParserData;

struct _ParserData
{
  gchar                *root;

  GList                *playlists;
  BlConfig             *config;
  guchar               *buffer;
  BModulePaintCallback  paint_callback;
  gpointer              paint_data;

  BlPlaylistItem       *item;
};


static BParserState  parser_start_element  (BParserState     state,
                                            const gchar     *element_name,
                                            const gchar    **attribute_names,
                                            const gchar    **attribute_values,
                                            gpointer         user_data,
                                            GError         **error);
static BParserState  parser_end_element    (BParserState     state,
                                            const gchar     *element_name,
                                            const gchar     *cdata,
                                            gsize            cdata_len,
                                            gpointer         user_data,
                                            GError         **error);
static GType         item_parse_attributes (BlPlaylistItem  *item,
                                            const gchar    **names,
                                            const gchar    **values);


gboolean
bl_playlist_parse (BlPlaylist            *playlist,
                   BModulePaintCallback   paint_callback,
                   gpointer               paint_data,
                   GError               **error)
{
  BParser     *parser;
  ParserData   data = { 0 };
  GIOChannel  *io;
  const gchar *filename;
  gboolean     retval;

  g_return_val_if_fail (BL_IS_PLAYLIST (playlist), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  filename = b_object_get_filename (B_OBJECT (playlist));

  g_return_val_if_fail (filename != NULL, FALSE);

  io = g_io_channel_new_file (filename, "r", error);
  if (! io)
    return FALSE;

  data.playlists      = g_list_prepend (NULL, playlist);
  data.config         = playlist->config;
  data.buffer         = playlist->buffer;
  data.paint_callback = paint_callback;
  data.paint_data     = paint_data;

  if (g_path_is_absolute (filename))
    {
      data.root = g_path_get_dirname (filename);
    }
  else
    {
      gchar *dir = g_get_current_dir ();
      gchar *tmp = g_build_filename (dir, filename, NULL);

      data.root = g_path_get_dirname (tmp);

      g_free (tmp);
      g_free (dir);
    }

  parser = b_parser_new (parser_start_element, parser_end_element, &data);

  retval = b_parser_parse_io_channel (parser, io, TRUE, error);

  if (retval && b_parser_get_state (parser) != PARSER_FINISH)
    {
      g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                   "This doesn't look like a Blinkenlights playlist");
      retval = FALSE;
    }

  b_parser_free (parser);

  g_free (data.root);

  g_io_channel_unref (io);

  return retval;
}


/*  private functions  */

static inline void
parser_prepend_item (ParserData     *data,
                     BlPlaylistItem *item)
{
  BlPlaylist *playlist;

  playlist = BL_PLAYLIST (data->playlists->data);

  playlist->items = g_list_prepend (playlist->items, item);
}

static BParserState
parser_start_element (BParserState   state,
                      const gchar   *element_name,
                      const gchar  **attribute_names,
                      const gchar  **attribute_values,
                      gpointer       user_data,
                      GError       **error)
{
  ParserData *data = (ParserData *) user_data;

  switch (state)
    {
    case PARSER_IN_PLAYLIST:
      if (! strcmp (element_name, "item"))
        {
          GType module_type;

          data->item = bl_playlist_item_new ();

          module_type = item_parse_attributes (data->item,
                                               attribute_names,
                                               attribute_values);

          data->item->module = b_module_new (module_type,
                                             data->config->width,
                                             data->config->height,
                                             data->buffer,
                                             data->paint_callback,
                                             data->paint_data,
                                             NULL);
          if (data->item->module)
            b_module_set_aspect (data->item->module, data->config->aspect);

          return PARSER_IN_ITEM;
        }

      if (! strcmp (element_name, "list"))
        {
          /* just for storing the effects */
          data->item = bl_playlist_item_new ();

          /* ignore GType return value */
          item_parse_attributes (data->item,
                                 attribute_names,
                                 attribute_values);

          return PARSER_IN_LIST;
        }

      /* fallthru */

    case B_PARSER_STATE_TOPLEVEL:
      if (! strcmp (element_name, "playlist"))
        {
          BlPlaylist     *playlist;
          BlPlaylistItem *item;
          gint            i;

          if (state == PARSER_IN_PLAYLIST)
            {
              playlist = bl_playlist_new (data->config);
              data->playlists = g_list_prepend (data->playlists, playlist);
            }
          else
            {
              playlist = BL_PLAYLIST (data->playlists->data);
            }

          for (i = 0; attribute_names[i] && attribute_values[i]; i++)
            {
              if (! strcmp (attribute_names[i], "shuffle"))
                b_parse_boolean (attribute_values[i], &playlist->shuffle);
             }

          /* just for storing the effects */
          item = bl_playlist_item_new ();

          /* ignore GType return value */
          item_parse_attributes (item,
                                 attribute_names,
                                 attribute_values);

          g_object_set_data_full (G_OBJECT (playlist), "effect-item", item,
                                  (GDestroyNotify) g_object_unref);

          return PARSER_IN_PLAYLIST;
        }
      break;

    case PARSER_IN_ITEM:
      if (! strcmp (element_name, "param") && data->item->module)
        {
          if (!b_parse_param (G_OBJECT (data->item->module), data->root,
                              attribute_names, attribute_values, error))
            {
              g_object_unref (data->item->module);
              data->item->module = NULL;
              break;
            }
          return PARSER_IN_PARAM;
        }
      break;

    default:
      break;
    }

  return B_PARSER_STATE_UNKNOWN;
}

static BParserState
parser_end_element (BParserState   state,
                    const gchar   *element_name,
                    const gchar   *cdata,
                    gsize          cdata_len,
                    gpointer       user_data,
                    GError       **error)
{
  ParserData *data = (ParserData *) user_data;

  switch (state)
    {
    case PARSER_IN_PLAYLIST:
      {
        BlPlaylist     *playlist;
        BlPlaylistItem *item;

        playlist = BL_PLAYLIST (data->playlists->data);

        playlist->items = g_list_reverse (playlist->items);

        item = g_object_get_data (G_OBJECT (playlist), "effect-item");

        if (item)
          {
            bl_playlist_item_apply_effects (BL_PLAYLIST_ITEM (playlist),
                                            item->effects,
                                            item->loop,
                                            item->reverse,
                                            item->speed);

            g_object_set_data (G_OBJECT (playlist), "effect-item", NULL);
          }

        if (g_list_length (data->playlists) == 1)
          return PARSER_FINISH;

        data->playlists = g_list_remove (data->playlists, playlist);

        if (g_list_length (playlist->items) == 0)
          {
            g_object_unref (G_OBJECT (playlist));
          }
        else
          {
            parser_prepend_item (data, BL_PLAYLIST_ITEM (playlist));
          }
      }
      return PARSER_IN_PLAYLIST;

    case PARSER_IN_LIST:
      {
        BlPlaylist *sublist = NULL;

        if (cdata)
          {
            gchar *filename;

            filename = b_filename_from_utf8 (cdata, data->root, NULL);

            if (filename)
              {
                sublist = bl_playlist_new_from_file (filename,
                                                     data->config,
                                                     data->paint_callback,
                                                     data->paint_data);
                g_free (filename);

                if (sublist->items == NULL)
                  {
                    g_object_unref (G_OBJECT (sublist));
                    sublist = NULL;
                  }
              }
          }

        if (sublist)
          {
            bl_playlist_item_apply_effects (BL_PLAYLIST_ITEM (sublist),
                                            data->item->effects,
                                            data->item->loop,
                                            data->item->reverse,
                                            data->item->speed);

            parser_prepend_item (data, BL_PLAYLIST_ITEM (sublist));
          }

        g_object_unref (G_OBJECT (data->item));
      }
      data->item = NULL;
      return PARSER_IN_PLAYLIST;

    case PARSER_IN_ITEM:
      if (data->item->module)
        {
          if (data->item->speed != 1.0 || data->item->module->speed != 1.0)
            {
              data->item->speed *= data->item->module->speed;

              g_object_set (G_OBJECT (data->item->module),
                            "speed", data->item->speed,
                            NULL);
            }

          if (B_IS_MOVIE_PLAYER (data->item->module))
            {
              BMoviePlayer *player;

              player = B_MOVIE_PLAYER (data->item->module);

              if (data->item->reverse || player->reverse)
                {
                  data->item->reverse ^= player->reverse;

                  player->reverse = data->item->reverse;
                }
            }

          parser_prepend_item (data, data->item);
        }
      else
        {
          g_object_unref (G_OBJECT (data->item));
        }

      data->item = NULL;
      return PARSER_IN_PLAYLIST;

    case PARSER_IN_PARAM:
      return PARSER_IN_ITEM;

    default:
      break;
    }

  return B_PARSER_STATE_UNKNOWN;
}

static BEffectScope
parse_effect_scope (const gchar *value)
{
  if (value)
    {
      if (! g_ascii_strcasecmp (value, "left"))
        return B_EFFECT_SCOPE_LEFT;
      else if (! g_ascii_strcasecmp (value, "right"))
        return B_EFFECT_SCOPE_RIGHT;
    }

  return B_EFFECT_SCOPE_ALL;
}

static GType
item_parse_attributes (BlPlaylistItem  *item,
                       const gchar    **names,
                       const gchar    **values)
{
  const gchar *type_name = "BMoviePlayer";
  gint         i;

  for (i = 0; names[i] && values[i]; i++)
    {
      if (! strcmp (names[i], "type"))
        {
          type_name = values[i];
        }
      else if (! strcmp (names[i], "loop"))
        {
          gint loop;

          if (b_parse_int (values[i], &loop) && loop > 1)
            item->loop = loop;
        }
      else if (! strcmp (names[i], "reverse"))
        {
          item->reverse = TRUE;
        }
      else if (! strcmp (names[i], "speed"))
        {
          gdouble speed;

          if (b_parse_double (values[i], &speed) &&
              speed >= 0.01 && speed <= 100)
            item->speed = speed;
        }
      else if (! strcmp (names[i], "likely"))
        {
          gdouble likely;

          if (b_parse_double (values[i], &likely) &&
              likely >= 0.01 && likely <= 100)
            item->likely = likely;
        }
      else if (! strcmp (names[i], "invert"))
        {
          item->effects->invert = parse_effect_scope (values[i]);
        }
      else if (! strcmp (names[i], "hflip"))
        {
          item->effects->hflip = parse_effect_scope (values[i]);
        }
      else if (! strcmp (names[i], "vflip"))
        {
          item->effects->vflip = parse_effect_scope (values[i]);
        }
      else if (! strcmp (names[i], "mirror"))
        {
          gboolean left  = FALSE;
          gboolean right = FALSE;

          if (! g_ascii_strcasecmp (values[i], "left"))
            left = TRUE;
          else if (! g_ascii_strcasecmp (values[i], "right"))
            right = TRUE;

          if (left || right)
            {
              if (left)
                {
                  item->effects->lmirror = TRUE;
                  item->effects->rmirror = FALSE;
                }
              else
                {
                  item->effects->rmirror = TRUE;
                  item->effects->lmirror = FALSE;
                }
            }
        }
    }

  return g_type_from_name (type_name);
}

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

#include <blib/blib.h>

#include "bltypes.h"

#include "blplaylistitem.h"


static void    bl_playlist_item_class_init    (BlPlaylistItemClass *klass);
static void    bl_playlist_item_init          (BlPlaylistItem      *item);
static void    bl_playlist_item_finalize      (GObject             *object);

static gchar * bl_playlist_item_real_describe      (BlPlaylistItem *item);
static void    bl_playlist_item_real_apply_effects (BlPlaylistItem *item,
                                                    BEffects       *effects,
                                                    gint            loop,
                                                    gboolean        reverse,
                                                    gdouble         speed);


static BObjectClass *parent_class = NULL;


GType
bl_playlist_item_get_type (void)
{
  static GType item_type = 0;

  if (! item_type)
    {
      static const GTypeInfo item_info =
      {
        sizeof (BlPlaylistItemClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) bl_playlist_item_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BlPlaylistItem),
        0,              /* n_preallocs */
        (GInstanceInitFunc) bl_playlist_item_init,
      };

      item_type = g_type_register_static (B_TYPE_OBJECT,
                                          "BlPlaylistItem",
                                          &item_info, 0);
    }

  return item_type;
}

static void
bl_playlist_item_class_init (BlPlaylistItemClass *klass)
{
  GObjectClass *object_class;

  object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = bl_playlist_item_finalize;

  klass->describe        = bl_playlist_item_real_describe;
  klass->apply_effects   = bl_playlist_item_real_apply_effects;
}

static void
bl_playlist_item_init (BlPlaylistItem *item)
{
  item->module  = NULL;
  item->effects = b_effects_new ();
  item->loop    = 0;
  item->count   = 0;
  item->reverse = FALSE;
  item->speed   = 1.0;
  item->likely  = 1.0;
}

static void
bl_playlist_item_finalize (GObject *object)
{
  BlPlaylistItem *item = BL_PLAYLIST_ITEM (object);

  if (item->module)
    g_object_unref (item->module);

  if (item->effects)
    g_object_unref (item->effects);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gchar *
bl_playlist_item_real_describe (BlPlaylistItem *item)
{
  GString *str;

  g_return_val_if_fail (BL_IS_PLAYLIST_ITEM (item), NULL);

  str = g_string_new ("");

  if (item->module)
    {
      BModule *module = item->module;

      g_string_printf (str, "'%s'",
                       g_type_name (G_TYPE_FROM_INSTANCE (module)));

      if (B_IS_MOVIE_PLAYER (module))
        {
          BMovie *movie = B_MOVIE_PLAYER (module)->movie;

          if (movie)
            {
              g_string_append_printf (str, " (%s)",
                                      b_object_get_name (B_OBJECT (movie)));
            }
          else
            {
              g_string_append_printf (str, " (%s)",
                                      B_MOVIE_PLAYER (module)->filename);
            }
        }

      if (item->loop > 1)
        g_string_append_printf (str, " (%dx)", item->loop);
    }

  return g_string_free (str, FALSE);
}

void
bl_playlist_item_real_apply_effects (BlPlaylistItem *item,
                                     BEffects       *effects,
                                     gint            loop,
                                     gboolean        reverse,
                                     gdouble         speed)
{
  item->effects->invert  ^= effects->invert;
  item->effects->vflip   ^= effects->vflip;
  item->effects->hflip   ^= effects->hflip;
  item->effects->lmirror ^= effects->lmirror;
  item->effects->rmirror ^= effects->rmirror;

  if (loop > 0)
    item->loop = loop;

  item->reverse ^= reverse;
  item->speed   *= speed;

  if (item->module)
    {
      g_object_set (G_OBJECT (item->module), "speed", item->speed, NULL);

      if (B_IS_MOVIE_PLAYER (item->module))
        B_MOVIE_PLAYER (item->module)->reverse = item->reverse;
    }
}

BlPlaylistItem *
bl_playlist_item_new (void)
{
  BlPlaylistItem *item = g_object_new (BL_TYPE_PLAYLIST_ITEM, NULL);

  return item;
}

gchar *
bl_playlist_item_describe (BlPlaylistItem *item)
{
  g_return_val_if_fail (BL_IS_PLAYLIST_ITEM (item), NULL);

  return BL_PLAYLIST_ITEM_GET_CLASS (item)->describe (item);
}

void
bl_playlist_item_apply_effects (BlPlaylistItem *item,
                                BEffects       *effects,
                                gint            loop,
                                gboolean        reverse,
                                gdouble         speed)
{
  g_return_if_fail (BL_IS_PLAYLIST_ITEM (item));
  g_return_if_fail (B_IS_EFFECTS (effects));

  BL_PLAYLIST_ITEM_GET_CLASS (item)->apply_effects (item, effects,
                                                    loop, reverse, speed);
}

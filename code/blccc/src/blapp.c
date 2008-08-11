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

#include "blapp.h"


static void    bl_app_class_init (BlAppClass     *klass);
static void    bl_app_init       (BlApp          *item);
static void    bl_app_finalize   (GObject        *object);
static gchar * bl_app_describe   (BlPlaylistItem *item);

static GObjectClass *parent_class = NULL;

GType
bl_app_get_type (void)
{
  static GType app_type = 0;

  if (! app_type)
    {
      static const GTypeInfo app_info =
      {
        sizeof (BlAppClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) bl_app_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BlApp),
        0,              /* n_preallocs */
        (GInstanceInitFunc) bl_app_init,
      };

      app_type = g_type_register_static (BL_TYPE_PLAYLIST_ITEM,
                                         "BlApp", &app_info, 0);
    }

  return app_type;
}

static void
bl_app_class_init (BlAppClass *klass)
{
  GObjectClass        *object_class;
  BlPlaylistItemClass *item_class;

  object_class = G_OBJECT_CLASS (klass);
  item_class   = BL_PLAYLIST_ITEM_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = bl_app_finalize;

  item_class->describe = bl_app_describe;
}

static void
bl_app_init (BlApp *app)
{
  app->lines = NULL;
}

static void
bl_app_finalize (GObject *object)
{
  BlApp *app = BL_APP (object);

  if (app->name)
    {
      g_free (app->name);
      app->name = NULL;
    }

  if (app->number)
    {
      g_free (app->number);
      app->number = NULL;
    }

  if (app->lines)
    {
      g_list_free (app->lines);
      app->lines = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gchar *
bl_app_describe (BlPlaylistItem *item)
{
  BlApp *app = BL_APP (item);

  return g_strdup_printf ("%s (%s)", app->name, app->number);
}

static BlApp *
bl_app_new (GType         module_type,
            const gchar  *name,
            const gchar  *number,
            const gchar  *sound,
            const gchar  *sound_loop,
            gboolean      public,
            gboolean      disabled,
            gint          priority)
{
  BlApp *app = g_object_new (BL_TYPE_APP, NULL);

  app->name       = g_strdup (name);
  app->number     = g_strdup (number);
  app->sound      = g_strdup (sound);
  app->sound_loop = g_strdup (sound_loop);
  app->disabled   = disabled;
  app->public     = public;
  app->priority   = priority;

  BL_PLAYLIST_ITEM (app)->module = g_object_new (module_type, NULL);

  return app;
}

BlApp *
bl_app_new_from_attributes (const gchar  **names,
                            const gchar  **values,
                            const gchar   *root,
                            GError       **error)
{
  GType        module_type;
  const gchar *type       = NULL;
  const gchar *name       = NULL;
  const gchar *number     = NULL;
  const gchar *sound      = NULL;
  const gchar *sound_loop = NULL;
  gboolean     public     = FALSE;
  gboolean     disabled   = FALSE;
  gint         priority   = 0;
  gint         i;

  g_return_val_if_fail (names != NULL && values != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  for (i = 0; names[i] && values[i]; i++)
    {
      if (strcmp (names[i], "name") == 0)
        name = values[i];
      else if (strcmp (names[i], "number") == 0)
        number = values[i];
      else if (strcmp (names[i], "type") == 0)
        type = values[i];
      else if (strcmp (names[i], "sound") == 0)
        sound = values[i];
      else if (strcmp (names[i], "soundloop") == 0)
        sound_loop = values[i];
      else if (strcmp (names[i], "public") == 0)
        b_parse_boolean (values[i], &public);
      else if (strcmp (names[i], "disabled") == 0)
        b_parse_boolean (values[i], &disabled);
    }

  if (!name)
    {
      g_set_error (error, 0, 0,
                   "name argument missing in application element");
      return NULL;
    }
  if (!number)
    {
      g_set_error (error, 0, 0,
                   "number argument missing in application element");
      return NULL;
    }
  if (!type)
    {
      g_set_error (error, 0, 0,
                   "type argument missing in application element");
      return NULL;
    }

  module_type = g_type_from_name (type);
  if (!module_type || !g_type_is_a (module_type, B_TYPE_MODULE))
    {
      g_set_error (error, 0, 0,
                   "application type '%s' is not a BModule", type);
      return NULL;
    }

  return bl_app_new (module_type,
                     name,
                     number, sound, sound_loop, public, disabled, priority);
}

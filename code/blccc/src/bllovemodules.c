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

#include <string.h>

#include <blib/blib.h>

#include "bltypes.h"

#include "bllovemodules.h"


typedef enum
{
  PARSER_IN_LOVEMODULES = B_PARSER_STATE_USER,
  PARSER_IN_MODULE,
  PARSER_FINISH
} ParserState;


static void  bl_lovemodules_class_init (BlLovemodulesClass *klass);
static void  bl_lovemodules_init       (BlLovemodules      *view);
static void  bl_lovemodules_finalize   (GObject            *object);

static BParserState  parser_start_element (BParserState     state,
                                           const gchar     *name,
                                           const gchar    **names,
                                           const gchar    **attribute_values,
                                           gpointer         user_data,
                                           GError         **error);
static BParserState  parser_end_element   (BParserState     state,
                                           const gchar     *element_name,
                                           const gchar     *cdata,
                                           gsize            cdata_len,
                                           gpointer         user_data,
                                           GError         **error);


static BObjectClass *parent_class = NULL;


GType
bl_lovemodules_get_type (void)
{
  static GType lovemodules_type = 0;

  if (!lovemodules_type)
    {
      static const GTypeInfo lovemodules_info =
      {
        sizeof (BlLovemodulesClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) bl_lovemodules_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data     */
	sizeof (BlLovemodules),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) bl_lovemodules_init,
      };

      lovemodules_type = g_type_register_static (BL_TYPE_LOVELETTERS,
                                                 "BlLovemodules",
                                                 &lovemodules_info, 0);
    }

  return lovemodules_type;
}

static void
bl_lovemodules_class_init (BlLovemodulesClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = bl_lovemodules_finalize;
}

static void
bl_lovemodules_init (BlLovemodules *lovemodules)
{
  lovemodules->hash = g_hash_table_new_full (g_str_hash, g_str_equal,
                                             g_free, g_free);
}

static void
bl_lovemodules_finalize (GObject *object)
{
  BlLovemodules *lovemodules = BL_LOVEMODULES (object);

  if (lovemodules->hash)
    {
      g_hash_table_destroy (lovemodules->hash);
      lovemodules->hash = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

BlLovemodules *
bl_lovemodules_new_from_file (const gchar  *filename,
                              GError      **error)
{
  BlLovemodules *lovemodules;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (g_path_is_absolute (filename), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  lovemodules = BL_LOVEMODULES (g_object_new (BL_TYPE_LOVEMODULES,
                                              "filename", filename,
                                              NULL));

  if (! bl_loveletters_parse (BL_LOVELETTERS (lovemodules), error) ||
      ! bl_lovemodules_parse (lovemodules, error))
    {
      g_object_unref (lovemodules);
      return NULL;
    }

  return lovemodules;
}

gboolean
bl_lovemodules_parse (BlLovemodules  *lovemodules,
                      GError        **error)
{
  const gchar *filename;
  GIOChannel  *io;
  BParser     *parser;
  gboolean     success;

  g_return_val_if_fail (BL_IS_LOVEMODULES (lovemodules), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  filename = b_object_get_filename (B_OBJECT (lovemodules));
  if (!filename)
    return TRUE;

  g_return_val_if_fail (g_path_is_absolute (filename), FALSE);

  io = g_io_channel_new_file (filename, "r", error);
  if (! io)
    return FALSE;

  parser = b_parser_new (parser_start_element,
                         parser_end_element,
                         lovemodules);

  success = b_parser_parse_io_channel (parser, io, TRUE, error);

  if (success && b_parser_get_state (parser) != PARSER_FINISH)
    {
      g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                   "This doesn't look like a Blinkenlights lovemodules file.");
      success = FALSE;
    }

  b_parser_free (parser);

  return success;
}

const gchar *
bl_lovemodules_lookup (BlLovemodules *lovemodules,
                       const gchar   *id)
{
  g_return_val_if_fail (BL_IS_LOVEMODULES (lovemodules), NULL);

  return g_hash_table_lookup (lovemodules->hash, id);
}

static BParserState
parser_start_element (BParserState     state,
                      const gchar     *element_name,
                      const gchar    **names,
                      const gchar    **values,
                      gpointer         user_data,
                      GError         **error)
{
  BlLovemodules *modules = user_data;

  switch (state)
    {
    case B_PARSER_STATE_TOPLEVEL:
      if (! strcmp (element_name, "loveletters"))
        return PARSER_IN_LOVEMODULES;
      break;

    case PARSER_IN_LOVEMODULES:
      if (! strcmp (element_name, "module"))
        {
          const gchar *id     = NULL;
          const gchar *vanity = NULL;
          const gchar *type   = NULL;
          gint         i;

          for (i = 0; names[i] && values[i]; i++)
            {
              if (!id     && strcmp (names[i], "id") == 0)
                id = values[i];
              if (!vanity && strcmp (names[i], "vanity") == 0)
                vanity = values[i];
              if (!type   && strcmp (names[i], "type") == 0)
                type = values[i];
            }

          if (!id)
            {
              g_set_error (error, 0, 0,
                           "id attribute is missing for module element.");
              break;
            }
          if (!type)
            {
              g_set_error (error, 0, 0,
                           "type attribute is missing for module element.");
              break;
            }

          if (g_hash_table_lookup (modules->hash, id) ||
              bl_loveletters_lookup (BL_LOVELETTERS (modules), id))
            {
              g_set_error (error, 0, 0, "id '%s' is not unique.", id);
              break;
            }

          if (strcmp (id, "default"))
            bl_loveletter_validate (id, vanity);

          g_hash_table_insert (modules->hash, g_strdup (id), g_strdup (type));

          return PARSER_IN_MODULE;
        }
      break;

    default:
      break;
    }

  return B_PARSER_STATE_UNKNOWN;
}

static BParserState
parser_end_element (BParserState     state,
                    const gchar     *element_name,
                    const gchar     *cdata,
                    gsize            cdata_len,
                    gpointer         user_data,
                    GError         **error)
{
  switch (state)
    {
    case PARSER_IN_LOVEMODULES:
      return PARSER_FINISH;

    case PARSER_IN_MODULE:
      return PARSER_IN_LOVEMODULES;

    default:
      break;
    }

  return B_PARSER_STATE_UNKNOWN;
}

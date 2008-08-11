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

#include "blloveletters.h"


typedef enum
{
  PARSER_IN_LOVELETTERS = B_PARSER_STATE_USER,
  PARSER_IN_MOVIE,
  PARSER_FINISH
} ParserState;

typedef struct _ParserData ParserData;
struct _ParserData
{
  gchar      *root;
  GHashTable *hash;
};


static void  bl_loveletters_class_init (BlLovelettersClass *klass);
static void  bl_loveletters_init       (BlLoveletters      *view);
static void  bl_loveletters_finalize   (GObject            *object);

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

static const gchar *vanity_codes[] =
{
  "0+",
  "1",
  "2abc",
  "3def",
  "4ghi",
  "5jkl",
  "6mno",
  "7pqrs",
  "8tuv",
  "9wxyz",
};


GType
bl_loveletters_get_type (void)
{
  static GType loveletters_type = 0;

  if (!loveletters_type)
    {
      static const GTypeInfo loveletters_info =
      {
        sizeof (BlLovelettersClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) bl_loveletters_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data     */
	sizeof (BlLoveletters),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) bl_loveletters_init,
      };

      loveletters_type = g_type_register_static (B_TYPE_OBJECT,
                                                 "BlLoveletters",
                                                 &loveletters_info, 0);
    }

  return loveletters_type;
}

static void
bl_loveletters_class_init (BlLovelettersClass *klass)
{
  GObjectClass *object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = bl_loveletters_finalize;
}

static void
bl_loveletters_init (BlLoveletters *loveletters)
{
  loveletters->hash = g_hash_table_new_full (g_str_hash, g_str_equal,
                                             g_free, g_free);
}

static void
bl_loveletters_finalize (GObject *object)
{
  BlLoveletters *loveletters = BL_LOVELETTERS (object);

  if (loveletters->hash)
    {
      g_hash_table_destroy (loveletters->hash);
      loveletters->hash = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

BlLoveletters *
bl_loveletters_new_from_file (const gchar  *filename,
                              GError      **error)
{
  BlLoveletters *loveletters;

  g_return_val_if_fail (filename != NULL, NULL);
  g_return_val_if_fail (g_path_is_absolute (filename), NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  loveletters = BL_LOVELETTERS (g_object_new (BL_TYPE_LOVELETTERS,
                                              "filename", filename,
                                              NULL));

  if (! bl_loveletters_parse (loveletters, error))
    {
      g_object_unref (loveletters);
      return NULL;
    }

  return loveletters;
}

gboolean
bl_loveletters_parse (BlLoveletters  *loveletters,
                      GError        **error)
{
  const gchar *filename;
  GIOChannel  *io;
  BParser     *parser;
  ParserData   data;
  gboolean     success;

  g_return_val_if_fail (BL_IS_LOVELETTERS (loveletters), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  filename = b_object_get_filename (B_OBJECT (loveletters));
  if (!filename)
    return TRUE;

  g_return_val_if_fail (g_path_is_absolute (filename), FALSE);

  io = g_io_channel_new_file (filename, "r", error);
  if (! io)
    return FALSE;

  data.hash = loveletters->hash;
  data.root = g_path_get_dirname (filename);

  parser = b_parser_new (parser_start_element, parser_end_element, &data);

  success = b_parser_parse_io_channel (parser, io, TRUE, error);

  if (success && b_parser_get_state (parser) != PARSER_FINISH)
    {
      g_set_error (error, G_MARKUP_ERROR, G_MARKUP_ERROR_INVALID_CONTENT,
                   "This doesn't look like a Blinkenlights loveletters file.");
      success = FALSE;
    }

  b_parser_free (parser);
  g_free (data.root);

  return success;
}

const gchar *
bl_loveletters_lookup (BlLoveletters *loveletters,
                       const gchar   *id)
{
  g_return_val_if_fail (BL_IS_LOVELETTERS (loveletters), NULL);

  return g_hash_table_lookup (loveletters->hash, id);
}

void
bl_loveletter_validate (const gchar *id,
                        const gchar *vanity)
{
  const gchar *v_code = NULL;
  gint  id_len, v_len, i;

  id_len = id     ? strlen (id)     : 0;
  v_len  = vanity ? strlen (vanity) : 0;

  if (v_len && id_len != v_len)
    {
      g_printerr ("Loveletters-Warning: "
                  "id '%s' has invalid vanity code", id);
      vanity = NULL;
    }

  for (i = 0; i < id_len; i++)
    {
      switch (id[i])
        {
        case '0' ... '9':
          v_code = vanity_codes[id[i] - '0'];
          break;

        default:
          g_printerr ("Loveletters-Warning: "
                      "id '%s' contains illegal character(s).\n",id);
          return;
        }

      if (!vanity)
        continue;

      if (!strchr (v_code, g_ascii_tolower (vanity[i])))
        {
          g_printerr ("Loveletters-Warning: "
                      "id '%s' has invalid vanity code.\n", id);
          vanity = NULL;
        }
    }
}

static BParserState
parser_start_element (BParserState     state,
                      const gchar     *element_name,
                      const gchar    **names,
                      const gchar    **values,
                      gpointer         user_data,
                      GError         **error)
{
  ParserData *data = (ParserData *) user_data;

  switch (state)
    {
    case B_PARSER_STATE_TOPLEVEL:
      if (! strcmp (element_name, "loveletters"))
        return PARSER_IN_LOVELETTERS;
      break;

    case PARSER_IN_LOVELETTERS:
      if (! strcmp (element_name, "movie"))
        {
          const gchar *id     = NULL;
          const gchar *vanity = NULL;
          const gchar *href   = NULL;
          gchar       *filename;
          gint         i;

          for (i = 0; names[i] && values[i]; i++)
            {
              if (!id     && strcmp (names[i], "id") == 0)
                id = values[i];
              if (!vanity && strcmp (names[i], "vanity") == 0)
                vanity = values[i];
              if (!href   && strcmp (names[i], "href") == 0)
                href = values[i];
            }

          if (!id)
            {
              g_set_error (error, 0, 0,
                           "id attribute is missing for movie element.");
              break;
            }
          if (!href)
            {
              g_set_error (error, 0, 0,
                           "href attribute is missing for movie element.");
              break;
            }

          if (g_hash_table_lookup (data->hash, id))
            {
              g_set_error (error, 0, 0, "id '%s' is not unique.", id);
              break;
            }

          filename = b_filename_from_utf8 (href, data->root, error);
          if (filename)
            {
              if (!g_file_test (filename, G_FILE_TEST_EXISTS))
                g_printerr ("Loveletters-Warning: "
                            "href for '%s' points to nonexistant file.\n", id);

              bl_loveletter_validate (id, vanity);

              g_hash_table_insert (data->hash, g_strdup (id), filename);
              return PARSER_IN_MOVIE;
            }
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
    case PARSER_IN_LOVELETTERS:
      return PARSER_FINISH;

    case PARSER_IN_MOVIE:
      return PARSER_IN_LOVELETTERS;

    default:
      break;
    }

  return B_PARSER_STATE_UNKNOWN;
}

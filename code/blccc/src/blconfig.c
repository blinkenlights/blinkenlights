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
#include "blconfig.h"

#define DEFAULT_WIDTH       18
#define DEFAULT_HEIGHT      8
#define DEFAULT_TELNET_PORT 2323
#define DEFAULT_ISDN_PORT   1234
#define DEFAULT_ISDN_LINES  4

enum
{
  PROP_0,
  PROP_WIDTH,
  PROP_HEIGHT,
  PROP_MAXVAL,
  PROP_CHANNELS,
  PROP_ASPECT,
  PROP_PLAYLIST,
  PROP_LOGFILE,
  PROP_RECIPIENT,
  PROP_TELNET_PORT,
  PROP_ISDN_HOST,
  PROP_ISDN_PORT,
  PROP_ISDN_LISTEN,
  PROP_ISDN_LINES,
  PROP_AUTH_CALLER
};

static void  bl_config_class_init   (BlConfigClass  *klass);
static void  bl_config_init         (BlConfig       *view);
static void  bl_config_finalize     (GObject        *object);
static void  bl_config_set_property (GObject        *object,
                                     guint           property_id,
                                     const GValue   *value,
                                     GParamSpec     *pspec);

/* parser functions */

typedef struct _ParserData ParserData;

struct _ParserData
{
  BlConfig *config;
  BlApp    *app;
  gchar    *root;
};

enum
{
  PARSER_IN_BLCCC = B_PARSER_STATE_USER,
  PARSER_IN_CONFIG,
  PARSER_IN_PARAM,
  PARSER_IN_APP,
  PARSER_IN_APP_PARAM,
  PARSER_FINISH
};

static BParserState  parser_start_element (BParserState   state,
                                           const gchar   *element_name,
                                           const gchar  **attribute_names,
                                           const gchar  **attribute_values,
                                           gpointer       user_data,
                                           GError       **error);
static BParserState  parser_end_element   (BParserState   state,
                                           const gchar   *element_name,
                                           const gchar   *cdata,
                                           gsize          cdata_len,
                                           gpointer       user_data,
                                           GError       **error);


static BObjectClass *parent_class = NULL;


GType
bl_config_get_type (void)
{
  static GType config_type = 0;

  if (!config_type)
    {
      static const GTypeInfo config_info =
      {
        sizeof (BlConfigClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) bl_config_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data     */
	sizeof (BlConfig),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) bl_config_init,
      };

      config_type = g_type_register_static (B_TYPE_OBJECT, 
                                            "BlConfig", &config_info, 0);
    }
  
  return config_type;
}

static void
bl_config_class_init (BlConfigClass *klass)
{
  GObjectClass *object_class;
  GParamSpec   *param_spec;

  object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize     = bl_config_finalize;
  object_class->set_property = bl_config_set_property;

  param_spec = g_param_spec_int ("width", NULL, NULL,
                                 1, G_MAXSHORT, DEFAULT_WIDTH,
                                 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_WIDTH, param_spec);

  param_spec = g_param_spec_int ("height", NULL, NULL,
                                 1, G_MAXSHORT, DEFAULT_HEIGHT,
                                 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_HEIGHT, param_spec);

  param_spec = g_param_spec_int ("maxval", NULL, NULL,
                                 1, 255, 255,
                                 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_MAXVAL, param_spec);

  param_spec = g_param_spec_int ("channels", NULL, NULL,
                                 1, G_MAXSHORT, 1,
                                 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_CHANNELS, param_spec);

  param_spec = g_param_spec_double ("aspect", NULL, NULL,
                                    0.01, 100.0, 1.0,
                                    G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_ASPECT, param_spec);

  param_spec = b_param_spec_filename ("playlist", NULL, NULL,
                                      "playlist.default.xml",
                                      G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_PLAYLIST, param_spec);

  param_spec = b_param_spec_filename ("logfile", NULL, NULL,
                                      NULL,
                                      G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_LOGFILE, param_spec);

  param_spec = g_param_spec_string ("recipient", NULL, NULL,
                                    NULL,
                                    G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_RECIPIENT, param_spec);

  param_spec = g_param_spec_int ("telnet-port", NULL, NULL,
                                 1024, 65535, DEFAULT_TELNET_PORT,
                                 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_TELNET_PORT, param_spec);

  param_spec = g_param_spec_string ("isdn-host", NULL, NULL,
                                    NULL,
                                    G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_ISDN_HOST, param_spec);

  param_spec = g_param_spec_int ("isdn-port", NULL, NULL,
                                 1024, 65535, DEFAULT_ISDN_PORT,
                                 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_ISDN_PORT, param_spec);

  param_spec = g_param_spec_int ("isdn-listen", NULL, NULL,
                                 1024, 65535, DEFAULT_ISDN_PORT,
                                 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_ISDN_LISTEN, param_spec);

  param_spec = g_param_spec_int ("isdn-lines", NULL, NULL,
                                 0, 8, DEFAULT_ISDN_LINES,
                                 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_ISDN_LINES, param_spec);

  param_spec = g_param_spec_string ("authorized-caller", NULL, NULL,
                                    NULL,
                                    G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_AUTH_CALLER, param_spec);
}

static void
bl_config_init (BlConfig *config)
{
  config->recipients = NULL;
}

static void
bl_config_finalize (GObject *object)
{
  BlConfig *config = BL_CONFIG (object);

  if (config->recipients)
    {
      GList *list;

      for (list = config->recipients; list; list = list->next)
        g_free (list->data);

      g_list_free (config->recipients);
      config->recipients = NULL;
    }

  if (config->authorized_callers)
    {
      GList *list;

      for (list = config->authorized_callers; list; list = list->next)
        g_free (list->data);

      g_list_free (config->authorized_callers);
      config->authorized_callers = NULL;
    }

  if (config->applications)
    {
      GList *list;

      for (list = config->applications; list; list = list->next)
        g_object_unref (G_OBJECT (list->data));

      g_list_free (config->applications);
      config->applications = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
bl_config_set_property (GObject      *object,
                        guint         property_id,
                        const GValue *value,
                        GParamSpec   *pspec)
{
  BlConfig *config = BL_CONFIG (object);

  switch (property_id)
    {
    case PROP_WIDTH:
      config->width = g_value_get_int (value);
      break;
    case PROP_HEIGHT:
      config->height = g_value_get_int (value);
      break;
    case PROP_MAXVAL:
      config->maxval = g_value_get_int (value);
      break;
    case PROP_CHANNELS:
      config->channels = g_value_get_int (value);
      break;
    case PROP_ASPECT:
      config->aspect = g_value_get_double (value);
      break;
    case PROP_PLAYLIST:
      g_free (config->playlist);
      config->playlist = g_value_dup_string (value);
      break;
    case PROP_LOGFILE:
      g_free (config->logfile);
      config->logfile = g_value_dup_string (value);
      break;
    case PROP_RECIPIENT:
      if (g_value_get_string (value))
        config->recipients = g_list_append (config->recipients,
                                            g_value_dup_string (value));
      break;
    case PROP_TELNET_PORT:
      config->telnet_port = g_value_get_int (value);
      break;
    case PROP_ISDN_HOST:
      config->isdn_host = g_value_dup_string (value);
      break;
    case PROP_ISDN_PORT:
      config->isdn_port = g_value_get_int (value);
      break;
    case PROP_ISDN_LISTEN:
      config->isdn_listen = g_value_get_int (value);
      break;
    case PROP_ISDN_LINES:
      config->isdn_lines = g_value_get_int (value);
      break;
    case PROP_AUTH_CALLER:
      if (g_value_get_string (value))
        config->authorized_callers = g_list_append (config->authorized_callers,
                                                    g_value_dup_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

BlConfig *
bl_config_new (void)
{
  return BL_CONFIG (g_object_new (BL_TYPE_CONFIG, NULL));
}

gboolean
bl_config_parse (BlConfig     *config,
                 const gchar  *filename,
                 GError      **error)
{
  GIOChannel *io;
  BParser    *parser;
  ParserData  data;
  GList      *list;
  gboolean    retval;

  g_return_val_if_fail (BL_IS_CONFIG (config), FALSE);
  g_return_val_if_fail (filename != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  g_object_set (G_OBJECT (config), "filename", filename, NULL);

  io = g_io_channel_new_file (filename, "r", error);
  if (! io)
    return FALSE;

  data.config = config;
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
                   "This doesn't look like a blccc configuration file.");
      retval = FALSE;
    }

  b_parser_free (parser);
  g_free (data.root);
  g_io_channel_unref (io);

  /* now query the registered applications */
  for (list = config->applications; list; list = list->next)
    {
      BlApp *app           = BL_APP (list->data);
      BlPlaylistItem *item = BL_PLAYLIST_ITEM (app);

      if (B_MODULE_GET_CLASS (item->module)->query (config->width,
                                                    config->height,
                                                    1, 255))
        {
          g_print ("Registered app: %s on %s %s %s\n",
                   app->name, app->number,
                   app->public   ? "(public)"   : "",
                   app->disabled ? "(disabled)" : "");
        }
      else
        {
          g_set_error (error, 0, 0,
                       "Application %s can not handle this configuration.",
                       app->name);
          return FALSE;
        }
    }

  return retval;
}

gboolean
bl_config_authorize_caller (BlConfig    *config,
                            const gchar *caller)
{
  GList *list;

  g_return_val_if_fail (BL_IS_CONFIG (config), FALSE);

  if (!caller)
    return FALSE;
  
  for (list = config->authorized_callers; list; list = list->next)
    if (strcmp (caller, (gchar *) list->data) == 0)
      return TRUE;
  
  return FALSE;
}

BlApp *
bl_config_select_app (BlConfig    *config,
                      const gchar *called_number)
{
  GList *list;

  for (list = config->applications; list; list = list->next)
    {
      BlApp *app = list->data;

      if (strcmp (app->number, called_number) == 0)
        return app;
    }

  return NULL;
}

static BParserState
parser_start_element (BParserState   state,
                      const gchar   *element_name,
                      const gchar  **attribute_names,
                      const gchar  **attribute_values,
                      gpointer       user_data,
                      GError       **error)
{
  ParserData *data = user_data;

  switch (state)
    {
    case B_PARSER_STATE_TOPLEVEL:
      if (strcmp (element_name, "blccc") == 0)
        return PARSER_IN_BLCCC;
      break;

    case PARSER_IN_BLCCC:
      if (strcmp (element_name, "config") == 0)
        return PARSER_IN_CONFIG;
      break;

    case PARSER_IN_CONFIG:
      if (strcmp (element_name, "param") == 0)
        {
          if (b_parse_param (G_OBJECT (data->config), data->root,
                             attribute_names, attribute_values, error))
            return PARSER_IN_PARAM;
        }
      else if (strcmp (element_name, "application") == 0)
        {
          if ((data->app = bl_app_new_from_attributes (attribute_names,
                                                       attribute_values,
                                                       data->root, error)))
            return PARSER_IN_APP;
        }
      break;

    case PARSER_IN_APP:
      if (strcmp (element_name, "param") == 0)
        {
          if (b_parse_param (G_OBJECT (BL_PLAYLIST_ITEM (data->app)->module), 
                             data->root,
                             attribute_names, attribute_values,
                             error))
            return PARSER_IN_APP_PARAM;
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
  ParserData *data = user_data;

  switch (state)
    {  
    case PARSER_IN_BLCCC:
      return PARSER_FINISH;

    case PARSER_IN_CONFIG:
      return PARSER_IN_BLCCC;

    case PARSER_IN_APP:
      g_return_val_if_fail (BL_IS_APP (data->app), B_PARSER_STATE_UNKNOWN);
      data->config->applications = g_list_append (data->config->applications,
                                                  data->app);
      data->app = NULL;
      return PARSER_IN_CONFIG;

    case PARSER_IN_PARAM:
      return PARSER_IN_CONFIG;

    case PARSER_IN_APP_PARAM:
      return PARSER_IN_APP;

    default:
      break;
    }

  return B_PARSER_STATE_UNKNOWN;
}

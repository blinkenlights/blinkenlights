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

#include <blib/blib.h>

#include "bltypes.h"

#include "bllovemodules.h"
#include "bldispatch.h"


enum
{
  PROP_0,
  PROP_LOVEMODULES
};

static void      bl_dispatch_class_init   (BlDispatchClass  *class);
static void      bl_dispatch_finalize     (GObject          *object);
static void      bl_dispatch_set_property (GObject          *object,
                                           guint             property_id,
                                           const GValue     *value,
                                           GParamSpec       *pspec);
static gboolean  bl_dispatch_query        (gint              width,
                                           gint              height,
                                           gint              channels,
                                           gint              maxval);
static gboolean  bl_dispatch_prepare      (BModule          *module,
                                           GError          **error);
static void      bl_dispatch_relax        (BModule          *module);
static void      bl_dispatch_start        (BModule          *module);
static void      bl_dispatch_stop         (BModule          *module);
static void      bl_dispatch_event        (BModule          *module,
                                           BModuleEvent     *event);
static void      bl_dispatch_describe     (BModule          *module,
                                           const gchar     **title,
                                           const gchar     **description,
                                           const gchar     **author);

static BModule * bl_dispatch_module_lookup (BlDispatch      *dispatch,
                                            const gchar     *code);
static BModule * bl_dispatch_module_new    (BlDispatch      *dispatch,
                                            const gchar     *type,
                                            GError         **error);


static BModuleClass *parent_class = NULL;


GType
bl_dispatch_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      static const GTypeInfo info =
      {
        sizeof (BlDispatchClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) bl_dispatch_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BlDispatch),
        0,              /* n_preallocs */
        NULL            /* instance_init */
      };

      type = g_type_register_static (B_TYPE_MODULE, "BlDispatch", &info, 0);
    }

  return type;
}

static void
bl_dispatch_class_init (BlDispatchClass *klass)
{
  GObjectClass      *object_class;
  BModuleClass      *module_class;
  GParamSpec        *param_spec;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize     = bl_dispatch_finalize;
  object_class->set_property = bl_dispatch_set_property;

  param_spec = b_param_spec_filename ("loveletters", NULL,
                                      "The filename of the loveletters / modules list.",
                                      NULL,
                                      G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_LOVEMODULES, param_spec);

  module_class->max_players = 1;

  module_class->query    = bl_dispatch_query;
  module_class->prepare  = bl_dispatch_prepare;
  module_class->relax    = bl_dispatch_relax;
  module_class->start    = bl_dispatch_start;
  module_class->stop     = bl_dispatch_stop;
  module_class->event    = bl_dispatch_event;
  module_class->describe = bl_dispatch_describe;
}

static void
bl_dispatch_finalize (GObject *object)
{
  BlDispatch *dispatch = BL_DISPATCH (object);

  if (dispatch->filename)
    {
      g_free (dispatch->filename);
      dispatch->filename = NULL;
    }
  if (dispatch->module)
    {
      g_object_unref (dispatch->module);
      dispatch->module = NULL;
    }
  if (dispatch->devices)
    {
      g_list_free (dispatch->devices);
      dispatch->devices = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
bl_dispatch_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  BlDispatch *dispatch = BL_DISPATCH (object);

  switch (property_id)
    {
    case PROP_LOVEMODULES:
      if (dispatch->filename)
        g_free (dispatch->filename);
      dispatch->filename = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
bl_dispatch_query (gint  width,
                   gint  height,
                   gint  channels,
                   gint  maxval)
{
  return (width > 2 * MAX_DISPATCH_CHARS && height > 3 &&
          channels == 1 && maxval == 255);
}

static gboolean
bl_dispatch_prepare (BModule  *module,
                     GError  **error)
{
  BlDispatch *dispatch = BL_DISPATCH (module);

  if (!dispatch->filename)
    {
      g_set_error (error, 0, 0, "No lovemodules file configured");
      return FALSE;
    }

  dispatch->lovemodules =
    bl_lovemodules_new_from_file (dispatch->filename, error);

  if (!dispatch->lovemodules)
    return FALSE;

  g_return_val_if_fail (dispatch->module == NULL, FALSE);

  dispatch->module = bl_dispatch_module_lookup (dispatch, "default");

  if (dispatch->module)
    {
      if (! b_module_prepare (dispatch->module, error))
        {
          g_object_unref (dispatch->module);
          dispatch->module = NULL;

          return FALSE;
        }

      /* hack! */
      B_MODULE_GET_CLASS (dispatch)->max_players =
        B_MODULE_GET_CLASS (dispatch->module)->max_players;
    }

  return TRUE;
}

static void
bl_dispatch_relax (BModule *module)
{
  BlDispatch *dispatch = BL_DISPATCH (module);

  if (dispatch->module)
    {
      b_module_relax (dispatch->module);

      g_object_unref (dispatch->module);
      dispatch->module = NULL;

      /* hack! */
      B_MODULE_GET_CLASS (dispatch)->max_players = 1;
    }
  if (dispatch->lovemodules)
    {
      g_object_unref (dispatch->lovemodules);
      dispatch->lovemodules = NULL;
    }
}

static void
bl_dispatch_start (BModule *module)
{
  BlDispatch *dispatch = BL_DISPATCH (module);

  dispatch->chars = -1;

  if (dispatch->module)
    {
      b_module_start (dispatch->module);
    }
}

static void
bl_dispatch_stop (BModule *module)
{
  BlDispatch *dispatch = BL_DISPATCH (module);

  if (dispatch->module && dispatch->module->running)
    b_module_stop (dispatch->module);
}

static gboolean
bl_dispatch_event_first (BModule      *module,
                         BModuleEvent *event)
{
  BlDispatch *dispatch = BL_DISPATCH (module);
  BModule    *new;

  switch (event->type)
    {
    case B_EVENT_TYPE_UNKNOWN:
      break;

    case B_EVENT_TYPE_PLAYER_ENTERED:
      dispatch->devices = g_list_append (dispatch->devices,
                                         GINT_TO_POINTER (event->device_id));
      break;

    case B_EVENT_TYPE_PLAYER_LEFT:
      dispatch->devices = g_list_remove (dispatch->devices,
                                         GINT_TO_POINTER (event->device_id));
      break;

    case B_EVENT_TYPE_KEY:
      switch (event->key)
        {
        case B_KEY_0 ... B_KEY_9:
          if (dispatch->chars < 0)
            break;

          dispatch->code[dispatch->chars] = event->key + '0';
          dispatch->chars++;

          if (dispatch->chars < MAX_DISPATCH_CHARS)
            break;
          /* else fallthru */

        case B_KEY_HASH:
          if (dispatch->chars <= 0)
            break;

          dispatch->code[dispatch->chars] = '\0';
          dispatch->chars = -1;

          new = bl_dispatch_module_lookup (dispatch, dispatch->code);

          if (new)
            {
              GList *list;

              if (dispatch->module)
                {
                  BModule *old = dispatch->module;

                  dispatch->module  = NULL;

                  b_module_stop (old);
                  b_module_relax (old);
                  g_object_unref (old);
                }

              dispatch->module = new;

              b_module_prepare (new, NULL);
              b_module_start (new);

              for (list = dispatch->devices; list; list = list->next)
                {
                  BModuleEvent event;

                  event.device_id = GPOINTER_TO_INT (list->data);
                  event.type      = B_EVENT_TYPE_PLAYER_ENTERED;

                  b_module_event (dispatch->module, &event);
                }
            }
          return TRUE; /* event is handled */

        case B_KEY_ASTERISK:
          dispatch->chars = 0;
          return TRUE; /* event is handled */
        }
      break;
    }

  return FALSE;
}

static void
bl_dispatch_event (BModule      *module,
                   BModuleEvent *event)
{
  BlDispatch *dispatch = BL_DISPATCH (module);

  if (bl_dispatch_event_first (module, event))
    return;

  if (dispatch->module)
    b_module_event (dispatch->module, event);
}

static void
bl_dispatch_describe (BModule      *module,
                      const gchar **title,
                      const gchar **description,
                      const gchar **author)
{
  BlDispatch *dispatch = BL_DISPATCH (module);

  if (dispatch->module && B_MODULE_GET_CLASS (dispatch->module)->describe)
    {
      B_MODULE_GET_CLASS (dispatch->module)->describe (dispatch->module,
                                                       title,
                                                       description,
                                                       author);
    }
  else
    {
      *title       = "BlDispatch";
      *author      = "Sven Neumann";
      *description = "Interactive module dispatcher";
    }
}

static BModule *
bl_dispatch_module_lookup (BlDispatch   *dispatch,
                           const gchar  *code)
{
  BModule     *module = NULL;
  const gchar *type;
  const gchar *movie = NULL;
  GError      *error = NULL;

  type = bl_lovemodules_lookup (dispatch->lovemodules, code);

  if (!type)
    {
      movie = bl_loveletters_lookup (BL_LOVELETTERS (dispatch->lovemodules),
                                     code);

      if (movie)
        type = "BMoviePlayer";
    }

  if (!type)
    return NULL;

  module = bl_dispatch_module_new (dispatch, type, &error);

  if (!module)
    {
      g_printerr (error->message);
      g_error_free (error);

      return NULL;
    }

  if (movie)
    {
      g_object_set (module, "movie", movie, NULL);

      g_signal_connect (module, "stop",
                        G_CALLBACK (b_module_start),
                        NULL);
    }

  return module;
}

static BModule *
bl_dispatch_module_new (BlDispatch   *dispatch,
                        const gchar  *type,
                        GError      **error)
{
  BModule *module      = B_MODULE (dispatch);
  GType    module_type = g_type_from_name (type);

  if (module_type && g_type_is_a (module_type, B_TYPE_MODULE))
    {
      BModule *new = b_module_new (module_type,
                                   module->width, module->height,
                                   module->buffer,
                                   module->paint_callback, module->paint_data,
                                   error);

      if (!new)
        return NULL;

      b_module_set_aspect (new, module->aspect);

      return new;
    }
  else
    {
      g_set_error (error, 0, 0, "%s is not a BModule", type);
      return NULL;
    }
}


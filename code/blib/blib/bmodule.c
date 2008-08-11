/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
 *                          Daniel Mack <daniel@yoobay.net>
 *                          Sven Neumann <sven@gimp.org>
 *                          Michael Natterer <mitch@gimp.org>
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

#include "config.h"

#include <glib-object.h>

#include "btypes.h"
#include "bmodule.h"
#include "bmodule-internal.h"

enum
{
  START,
  STOP,
  LAST_SIGNAL
};

enum
{
  PROP_0,
  PROP_SPEED,
  PROP_LIFETIME
};

static void   b_module_class_init   (BModuleClass  *klass);
static void   b_module_init         (BModule       *module);
static void   b_module_finalize     (GObject       *object);
static void   b_module_set_property (GObject       *object,
                                     guint          property_id,
                                     const GValue  *value,
                                     GParamSpec    *pspec);

static gint   tick                  (BModule       *module);
static void   describe              (BModule       *module,
                                     const gchar  **title,
                                     const gchar  **description,
                                     const gchar  **author);
static void   stop_callback         (BModule       *module);


static guint         module_signals[LAST_SIGNAL] = { 0 };
static GObjectClass *parent_class                = NULL;


GType
b_module_get_type (void)
{
  static GType module_type = 0;

  if (!module_type)
    {
      static const GTypeInfo module_info =
      {
        sizeof (BModuleClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_module_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BModule),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_module_init,
      };

      module_type = g_type_register_static (G_TYPE_OBJECT,
                                            "BModule", &module_info, 0);
    }

  return module_type;
}

static void
b_module_class_init (BModuleClass *klass)
{
  GObjectClass *object_class;
  GParamSpec   *param_spec;

  object_class = G_OBJECT_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize     = b_module_finalize;
  object_class->set_property = b_module_set_property;

  module_signals[START] = g_signal_new ("start",
                                        G_TYPE_FROM_CLASS (klass),
                                        G_SIGNAL_RUN_FIRST,
                                        G_STRUCT_OFFSET (BModuleClass, start),
                                        NULL, NULL,
                                        g_cclosure_marshal_VOID__VOID,
                                        G_TYPE_NONE, 0);
  module_signals[STOP] = g_signal_new ("stop",
                                       G_TYPE_FROM_CLASS (klass),
                                       G_SIGNAL_RUN_FIRST,
                                       G_STRUCT_OFFSET (BModuleClass, stop),
                                       NULL, NULL,
                                       g_cclosure_marshal_VOID__VOID,
                                       G_TYPE_NONE, 0);

  param_spec = g_param_spec_double ("speed", NULL,
                                    "Allows to tune the playback speed.",
                                    0.01, 100.0, 1.0,
                                    G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_SPEED, param_spec);

  param_spec = g_param_spec_int ("lifetime", NULL,
                                 "Maximum time a module is allowed to run "
                                 "(in milliseconds).",
                                 0, G_MAXINT, 0,
                                 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_LIFETIME, param_spec);

  klass->max_players = 0;

  klass->query    = NULL;
  klass->prepare  = NULL;
  klass->start    = NULL;
  klass->stop     = NULL;
  klass->event    = NULL;
  klass->tick     = NULL;
  klass->describe = describe;
}

static void
b_module_init (BModule *module)
{
  module->aspect         = 1.0;

  module->speed          = 1.0;
  module->lifetime       = 0;

  module->num_players    = 0;

  module->ready          = FALSE;
  module->running        = FALSE;
  module->tick_source_id = 0;
  module->life_source_id = 0;

  g_signal_connect (G_OBJECT (module), "stop",
                    G_CALLBACK (stop_callback),
                    NULL);
}

static void
b_module_finalize (GObject *object)
{
  BModule *module = B_MODULE (object);

  if (module->ready)
    b_module_relax (module);

  if (module->tick_source_id)
    {
      g_source_remove (module->tick_source_id);
      module->tick_source_id = 0;  
    }
  if (module->life_source_id)
    {
      g_source_remove (module->life_source_id);
      module->life_source_id = 0;  
    }
  if (module->owns_buffer && module->buffer)
    {
      g_free (module->buffer);
      module->buffer = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
b_module_set_property (GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  BModule *module = B_MODULE (object);

  switch (property_id)
    {
    case PROP_SPEED:
      g_return_if_fail (g_value_get_double (value) > 0.0);
      module->speed = g_value_get_double (value);
      break;

    case PROP_LIFETIME:
      module->lifetime = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
tick (BModule *module)
{
  gint timeout;

  g_object_ref (module);
  
  timeout = (gdouble) b_module_tick (module) / module->speed;

  if (timeout > 0)
    module->tick_source_id = g_timeout_add (timeout,
                                            (GSourceFunc) tick, module);
  else
    module->tick_source_id = 0;

  g_object_unref (module);

  return FALSE;
}

static gboolean
lifetime_expired (BModule *module)
{
  module->life_source_id = 0;

  b_module_stop (module);

  return FALSE;
}

static void
describe (BModule      *module,
          const gchar **title,
          const gchar **description,
          const gchar **author)
{
  *title       = g_type_name_from_instance ((GTypeInstance *) module);
  *description = NULL; 
  *author      = NULL;
}

static void
stop_callback (BModule *module)
{
  /*  this is a real signal callback, no virtual function implementation  */

  module->running = FALSE;
}


/***************************** 
 * internal module functions *
 *****************************/

/**
 * b_module_new:
 * @module_type: the type of module to create
 * @width: width of the frame buffer
 * @height: height of the frame buffer
 * @buffer: pointer to a preallocated buffer or %NULL 
 * @paint_callback: the function to call in b_module_paint()
 * @paint_data: data to pass to the @paint_callback
 * @error: location to store the error occuring, or %NULL to ignore errors
 * 
 * This function tries to create the class for the @module_type and
 * queries it with the given width and height. Only if the class can
 * handle the requested size, a #BModule instance is created and
 * initialized with the given values.
 * 
 * Return value: the newly allocate #BModule object
 **/
BModule *
b_module_new (GType                  module_type,
              gint                   width,
              gint                   height,
              guchar                *buffer,
              BModulePaintCallback   paint_callback,
              gpointer               paint_data,
              GError               **error)
{
  BModuleClass *klass;
  BModule      *module = NULL;

  g_return_val_if_fail (module_type != B_TYPE_MODULE, NULL);
  g_return_val_if_fail (g_type_is_a (module_type, B_TYPE_MODULE), NULL);
  g_return_val_if_fail (width > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);
  g_return_val_if_fail (paint_callback != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  klass = g_type_class_ref (module_type);

  if (!B_IS_MODULE_CLASS (klass))
    {
      g_set_error (error, 0, 0, 
                   "Class '%s' is not a BModuleClass.",
                   g_type_name (module_type));
    }
  else if (!klass->query || !klass->prepare)
    {
      g_set_error (error, 0, 0, 
                   "Module '%s' does not implement the BModule vtable.",
                   g_type_name (module_type));
    }
  else if (klass->query (width, height, 1, 255))
    {
      module = (BModule *) g_object_new (module_type, NULL);
    }
  else
    {
      g_set_error (error, 0, 0, 
                   "Module '%s' cannot handle the requested configuration.",
                   g_type_name (module_type));
    }

  g_type_class_unref (klass);

  if (!module)
    return NULL;

  if (buffer)
    {
      module->owns_buffer = FALSE;
    }
  else
    {
      buffer = g_new0 (guchar, width * height);
      module->owns_buffer = TRUE;
    }

  module->width          = width;
  module->height         = height;
  module->channels       = 1;
  module->maxval         = 255;
  module->buffer         = buffer;
  module->paint_callback = paint_callback;
  module->paint_data     = paint_data;

  return module;
}

/**
 * b_module_set_aspect:
 * @module: a #BModule object 
 * @aspect_ratio: the new pixel aspect ratio (x / y)
 * 
 * Sets the pixel (or window) aspect ratio for the @module. Most
 * modules ignore this value but some may adapt their output to take
 * the shape of pixels into account.
 **/
void
b_module_set_aspect (BModule *module,
                     gdouble  aspect_ratio)
{
  g_return_if_fail (B_IS_MODULE (module));
  g_return_if_fail (aspect_ratio >= 0.01 && aspect_ratio <= 100.0);

  module->aspect = aspect_ratio;
}

/**
 * b_module_prepare:
 * @module: a #BModule object
 * @error: location to store the error occuring, or %NULL to ignore errors
 * 
 * This function first queries the module once more to check that it
 * can handle the current settings. If the query succeeds, the prepare()
 * method of the module is called. The module should then prepare
 * itself and will be able to start as soon as b_module_start() is called.
 *
 * Return value: %TRUE is the module has successfully prepared itself,
 * %FALSE otherwise
 **/
gboolean
b_module_prepare (BModule  *module,
                  GError  **error)
{
  g_return_val_if_fail (B_IS_MODULE (module), FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);
  g_return_val_if_fail (module->ready == FALSE, FALSE);

  /* first query the module again with the current values */
  module->ready = B_MODULE_GET_CLASS (module)->query (module->width,
                                                      module->height,
                                                      module->channels,
                                                      module->maxval);

  if (!module->ready)
    {
      g_set_error (error, 0, 0, "Module can not handle this configuration.");
      return FALSE;
    }

  module->ready = B_MODULE_GET_CLASS (module)->prepare (module, error);

  if (!module->ready && error && *error == NULL)
    g_set_error (error, 0, 0, "Module gave no reason.");

  return module->ready;
}

/**
 * b_module_relax:
 * @module: a #BModule object
 * 
 * Calls the relax() method of the @module causing it to release
 * resources allocated in b_module_prepare().
 **/
void
b_module_relax (BModule *module)
{
  g_return_if_fail (B_IS_MODULE (module));
  g_return_if_fail (module->ready == TRUE);

  if (B_MODULE_GET_CLASS (module)->relax)
    B_MODULE_GET_CLASS (module)->relax (module);

  module->ready = FALSE;
}

/**
 * b_module_start:
 * @module: a #BModule object
 * 
 * Emits the start signal for @module. If @module has a lifetime set,
 * a timer is installed that stops the module when the lifetime expires.
 * 
 * You need to prepare @module by calling b_module_prepare() before it
 * can be started.
 **/
void
b_module_start (BModule *module)
{
  g_return_if_fail (B_IS_MODULE (module));
  g_return_if_fail (module->ready == TRUE);
  g_return_if_fail (module->running == FALSE);

  module->running = TRUE;

  g_signal_emit (G_OBJECT (module), module_signals[START], 0);

  if (module->lifetime > 0)
    module->life_source_id = g_timeout_add (module->lifetime,
                                            (GSourceFunc) lifetime_expired,
                                            module);
}

/**
 * b_module_stop:
 * @module: a #BModule object
 *
 * Emits the stop signal for @module. You may only call this function
 * for a #BModule that is currently running.
 **/
void
b_module_stop (BModule *module)
{
  g_return_if_fail (B_IS_MODULE (module));
  g_return_if_fail (module->running == TRUE);

  if (module->life_source_id)
    {
      g_source_remove (module->life_source_id);
      module->life_source_id = 0;
    }

  b_module_ticker_stop (module);

  g_signal_emit (G_OBJECT (module), module_signals[STOP], 0);
}

/**
 * b_module_event:
 * @module: a #BModule object
 * @event: pointer to a #BModuleEvent
 *
 * Dispatches an event to @module by calling its event() method with
 * @event. This function has no effect if the module is not currently
 * running.
 **/
void
b_module_event (BModule      *module,
                BModuleEvent *event)
{
  g_return_if_fail (B_IS_MODULE (module));
  g_return_if_fail (event != NULL);

  if (!module->running)
    return;

  if (B_MODULE_GET_CLASS (module)->event)
    B_MODULE_GET_CLASS (module)->event (module, event);
}

/**
 * b_module_tick:
 * @module: a #BModule object
 *
 * Calls the tick() method of @module.  You may only call this function
 * for a #BModule that is currently running.
 * 
 * Return value: the number of milliseconds until tick() should be
 * called again or -1 to indicate that no further ticks are requested
 **/
gint
b_module_tick (BModule *module)
{
  g_return_val_if_fail (B_IS_MODULE (module), -1);
  g_return_val_if_fail (module->running == TRUE, -1);

  if (!B_MODULE_GET_CLASS (module)->tick)
    return -1;

  return B_MODULE_GET_CLASS (module)->tick (module);
}

/**
 * b_module_describe:
 * @module: a #BModule object
 * @title: return location for the module title or %NULL
 * @description: return location for the module description or %NULL
 * @author: return location for the name of the module author or %NULL
 * 
 * This function queries @module for a title, description and the name
 * of the author. It does so by calling the modules describe() method.
 * You may pass %NULL as return location if you are not interested in
 * a particular information. The @title is guaranteed to be available,
 * @description and @author may be %NULL if the module doesn't provide
 * this information.
 * 
 * You must free all returned strings using g_free() if you don't need
 * them any longer.
 **/
void
b_module_describe (BModule  *module,
                   gchar   **title,
                   gchar   **description,
                   gchar   **author)
{
  const gchar *t, *d, *a;

  g_return_if_fail (B_IS_MODULE (module));

  B_MODULE_GET_CLASS (module)->describe (module, &t, &d, &a);

  if (!t)
    {
      g_warning ("Module %s didn't return a title in b_module_describe()!",
                 g_type_name_from_instance ((GTypeInstance *) module));
      t = g_type_name_from_instance ((GTypeInstance *) module);
    }

  if (title)
    *title = g_strdup (t);

  if (description)
    *description = g_strdup (d);

  if (author)
    *author = g_strdup (a);
}


/*************************** 
 * public module functions *
 ***************************/

/**
 * b_module_ticker_start:
 * @module: a #BModule object
 * @timeout: the timeout interval in milliseconds 
 * 
 * Starts a timeout for @module that will call cause its tick() method
 * to be called in @timeout milliseconds. The return value of the
 * tick() method is then used to install a new timeout for @module.
 * This proceeds until the tick() method returns -1,
 * b_module_ticker_stop() is called or the module is stopped.
 * 
 * You may only call this function for a #BModule that is currently
 * running.
 **/
void
b_module_ticker_start (BModule *module,
                       gint     timeout)
{
  g_return_if_fail (B_IS_MODULE (module));
  g_return_if_fail (B_MODULE_GET_CLASS (module)->tick);
  g_return_if_fail (module->running == TRUE);
  g_return_if_fail (timeout > 0);

  b_module_ticker_stop (module);

  timeout = (gdouble) timeout / module->speed;

  if (timeout > 0)
    module->tick_source_id = g_timeout_add (timeout,
                                            (GSourceFunc) tick, module);
}

/**
 * b_module_ticker_stop:
 * @module: a #BModule object
 * 
 * Stops the ticker for @module. If there is a pending timeout for
 * @module, it is removed.
 *
 * You may only call this function for a #BModule that is currently
 * running.
 **/
void
b_module_ticker_stop (BModule *module)
{
  g_return_if_fail (B_IS_MODULE (module));
  g_return_if_fail (module->running == TRUE);

  if (module->tick_source_id)
    {
      g_source_remove (module->tick_source_id);
      module->tick_source_id = 0;
    }
}

/**
 * b_module_request_stop:
 * @module: a #BModule object
 * 
 * This function causes the module to be stopped. This is for example
 * used when a game module decides that the game is over.
 *
 * You may only call this function for a #BModule that is currently
 * running.
 **/
void
b_module_request_stop (BModule *module)
{
  g_return_if_fail (B_IS_MODULE (module));
  g_return_if_fail (module->running == TRUE);

  b_module_stop (module);
}

/**
 * b_module_paint:
 * @module: a #BModule object
 * 
 * This function causes a repaint of the screen.
 **/
void
b_module_paint (BModule *module)
{
  g_return_if_fail (B_IS_MODULE (module));
  g_return_if_fail (module->running == TRUE);
  g_return_if_fail (module->paint_callback != NULL);

  if (!module->running)
    return;

  module->paint_callback (module, module->buffer, module->paint_data);
}

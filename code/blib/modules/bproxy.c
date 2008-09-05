/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2002  The Blinkenlights Crew
 *                     Daniel Mack <daniel@yoobay.net>
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

#include <stdlib.h>
#include <string.h>

#include <glib.h>
#include <gmodule.h>

#include <blib/blib.h>


#define B_TYPE_PROXY         (b_type_proxy)
#define B_PROXY(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_PROXY, BProxy))
#define B_PROXY_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_PROXY, BProxyClass))
#define B_IS_PROXY(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_PROXY))

typedef struct _BProxy      BProxy;
typedef struct _BProxyClass BProxyClass;

struct _BProxy
{
  BModule     parent_instance;

  BReceiver  *receiver;
  gint        port;
};

struct _BProxyClass
{
  BModuleClass  parent_class;
};

enum
{
  PROP_0,
  PROP_PORT
};


static GType    b_proxy_get_type     (GTypeModule   *module);
static void     b_proxy_class_init   (BProxyClass   *klass);
static void     b_proxy_finalize     (GObject       *object);
static void     b_proxy_set_property (GObject       *object,
				      guint          property_id,
				      const GValue  *value,
				      GParamSpec    *pspec);
static void     b_proxy_init         (BProxy        *proxy);
static gboolean b_proxy_query        (gint           width,
				      gint           height,
				      gint           channels,
				      gint           maxval);
static gboolean b_proxy_prepare      (BModule       *module,
				      GError       **error);
static void     b_proxy_relax        (BModule       *module);
static void     b_proxy_start        (BModule       *module);
static void     b_proxy_stop         (BModule       *module);
static void     b_proxy_describe     (BModule       *module,
				      const gchar  **title,
				      const gchar  **description,
				      const gchar  **author);


static BModuleClass * parent_class = NULL;
static GType          b_type_proxy = 0;

static gboolean
callback (BReceiver *rec,
          BPacket   *packet,
          gpointer   callback_data)
{
  BModule *module = B_MODULE (callback_data);

  if (packet->header.mcu_frame_h.width    == module->width  &&
      packet->header.mcu_frame_h.height   == module->height &&
      packet->header.mcu_frame_h.channels == 1              &&
      packet->header.mcu_frame_h.maxval   > 0)
    {
      gint max = packet->header.mcu_frame_h.maxval;

      if (max == 255)
        {
          memcpy (module->buffer, packet->data,
                  module->width * module->height);
        }
      else
        {
          gint bytes = module->width * module->height;
          guchar *s  = packet->data;
          guchar *d  = module->buffer;

          do
            {
              *d = ((gint) *s * 255) / max;
              s++, d++;
            }
          while (--bytes);
        }

      b_module_paint (module);
    }
  else
    {
      g_printerr ("BProxy: packet mismatch\n");
    }

  return TRUE;
}

G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_proxy_get_type (module);
  return TRUE;
}

GType
b_proxy_get_type (GTypeModule *module)
{
  if (!b_type_proxy)
    {
      static const GTypeInfo proxy_info =
      {
        sizeof (BProxyClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_proxy_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BProxy),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_proxy_init,
      };

      b_type_proxy = g_type_module_register_type (module,
						  B_TYPE_MODULE, "BProxy",
						  &proxy_info, 0);
    }

  return b_type_proxy;
}

static void
b_proxy_class_init (BProxyClass *klass)
{
  GObjectClass *object_class;
  BModuleClass *module_class;
  GParamSpec   *param_spec;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize     = b_proxy_finalize;
  object_class->set_property = b_proxy_set_property;

  param_spec = g_param_spec_int ("port", NULL,
                                 "IP Port to listen on.",
                                 1024, 65535, 4242,
                                 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_PORT, param_spec);

  module_class->query    = b_proxy_query;
  module_class->prepare  = b_proxy_prepare;
  module_class->relax    = b_proxy_relax;
  module_class->start    = b_proxy_start;
  module_class->stop     = b_proxy_stop;
  module_class->describe = b_proxy_describe;
}

static void
b_proxy_finalize (GObject *object)
{
  BProxy *proxy = B_PROXY (object);

  if (proxy->receiver)
    {
      g_object_unref (proxy->receiver);
      proxy->receiver = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
b_proxy_set_property (GObject      *object,
		      guint         property_id,
		      const GValue *value,
		      GParamSpec   *pspec)
{
  BProxy *proxy = B_PROXY (object);

  switch (property_id)
    {
    case PROP_PORT:
      g_return_if_fail (g_value_get_int (value) > 0);
      proxy->port = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
b_proxy_init (BProxy *proxy)
{
}

static gboolean
b_proxy_query (gint     width,
	       gint     height,
	       gint     channels,
	       gint     maxval)
{
  return (width > 0 && height > 0 && channels == 1 && maxval == 255);
}

static gboolean
b_proxy_prepare (BModule  *module,
		 GError  **error)
{
  BProxy *proxy = B_PROXY (module);

  proxy->receiver = b_receiver_new (callback, module);

  return TRUE;
}

static void
b_proxy_relax (BModule *module)
{
  BProxy *proxy = B_PROXY (module);

  if (proxy->receiver)
    {
      g_object_unref (proxy->receiver);
      proxy->receiver = NULL;
    }
}

static void
b_proxy_start (BModule *module)
{
  BProxy *proxy = B_PROXY (module);

  b_module_fill (module, 0);
  b_module_paint (module);

  if (b_receiver_listen (proxy->receiver, proxy->port))
    {
      g_printerr ("BProxy: listening on port %d\n", proxy->port);
    }
  else
    {
      g_printerr ("BProxy: can not listen on port %d, exiting\n", proxy->port);
      b_module_stop (module);
    }
}

static void
b_proxy_stop (BModule *module)
{
  b_receiver_stop (B_PROXY (module)->receiver);
}

static void
b_proxy_describe (BModule      *module,
		  const gchar **title,
		  const gchar **description,
		  const gchar **author)
{
  *title       = "BProxy";
  *description = "Generic network proxy";
  *author      = "Daniel Mack";
}

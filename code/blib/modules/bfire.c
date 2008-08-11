/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2002  The Blinkenlights Crew
 *                     Sven Neumann <sven@gimp.org>
 *
 * Fire algorithm is based upon aafire which is distributed with aalib.
 *
 *                              dT8  8Tb
 *                             dT 8  8 Tb
 *                            dT  8  8  Tb
 *                         <PROJECT><PROJECT>
 *                          dT    8  8    Tb
 *                         dT     8  8     Tb
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

#include <glib.h>
#include <gmodule.h>

#include <string.h>

#include <blib/blib.h>

#define MAXTABLE (256*5)
#define TIMEOUT  160

#define B_TYPE_FIRE         (b_type_fire)
#define B_FIRE(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_FIRE, BFire))
#define B_FIRE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_FIRE, BFireClass))
#define B_IS_FIRE(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_FIRE))

typedef struct _BFire      BFire;
typedef struct _BFireClass BFireClass;

struct _BFire
{
  BModule  parent_instance;

  gboolean flames;
  gint     frames;
  guint    table[MAXTABLE];
};

struct _BFireClass
{
  BModuleClass  parent_class;
};

enum
{
  PROP_0,
  PROP_FLAMES
};


static GType      b_fire_get_type     (GTypeModule  *module);

static void       b_fire_class_init   (BFireClass   *klass);
static void       b_fire_set_property (GObject      *object,
                                       guint         property_id,
                                       const GValue *value,
                                       GParamSpec   *pspec);
static void       b_fire_init         (BFire        *fire);

static gboolean   b_fire_query        (gint          width,
                                       gint          height,
                                       gint          channels,
                                       gint          maxval);
static gboolean   b_fire_prepare      (BModule      *module,
                                       GError      **error);
static void       b_fire_start        (BModule      *module);
static gint       b_fire_tick         (BModule      *module);
static void       b_fire_describe     (BModule      *module,
                                       const gchar **title,
                                       const gchar **description,
                                       const gchar **author);


static GType  b_type_fire = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_fire_get_type (module);
  return TRUE;
}

GType
b_fire_get_type (GTypeModule *module)
{
  if (!b_type_fire)
    {
      static const GTypeInfo fire_info =
      {
        sizeof (BFireClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_fire_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BFire),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_fire_init,
      };

      b_type_fire = g_type_module_register_type (module,
                                                 B_TYPE_MODULE, "BFire",
                                                 &fire_info, 0);
    }

  return b_type_fire;
}

static void
b_fire_class_init (BFireClass *klass)
{
  GObjectClass *object_class;
  BModuleClass *module_class;
  GParamSpec   *param_spec;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  object_class->set_property = b_fire_set_property;

  param_spec = g_param_spec_boolean ("flames", NULL,
                                     "Whether to draw flames or not.",
                                     TRUE,
                                     G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_FLAMES, param_spec);

  module_class->query    = b_fire_query;
  module_class->prepare  = b_fire_prepare;
  module_class->start    = b_fire_start;
  module_class->tick     = b_fire_tick;
  module_class->describe = b_fire_describe;
}

static void
b_fire_set_property (GObject      *object,
                     guint         property_id,
                     const GValue *value,
                     GParamSpec   *pspec)
{
  BFire *fire = B_FIRE (object);

  switch (property_id)
    {
    case PROP_FLAMES:
      fire->flames = g_value_get_boolean (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
b_fire_init (BFire *fire)
{
}

static gboolean
b_fire_query (gint     width,
              gint     height,
              gint     channels,
              gint     maxval)
{
  return (width > 0 && height > 3 && channels == 1 && maxval == 255);
}

static gboolean
b_fire_prepare (BModule  *module,
                GError  **error)
{
  BFire *fire = B_FIRE (module);
  guint  i, p2;
  gint   minus;

  g_return_val_if_fail (module->height > 0, FALSE);

  minus = MAX (1, 500 / module->height);

  for (i = 0; i < MAXTABLE; i++)
    {
      if (i > minus)
	{
	  p2 = (i - minus) / 5;
	  fire->table[i] = p2;
	}
      else
	fire->table[i] = 0;
    }
 
  return TRUE;
}

static void
b_fire_start (BModule *module)
{
  B_FIRE (module)->frames = 0;
  b_module_ticker_start (module, b_fire_tick (module));
}

static gint
b_fire_tick (BModule *module)
{
  BFire  *fire   = B_FIRE (module);
  gint    width  = module->width;
  gint    height = module->height;
  guchar *d;
  guint   i, r, last;

  if (fire->flames)
    {
      guint i1 = 1;
      guint i2 = 4 * width + 1;

      for (i = 0, d = module->buffer + width * (height - 3);
           i < width;
           i1 += 4, i2 -= 4, d++, i++)
        {
          for (r = rand() % 6, last = 4 * (rand() % MIN (i1, MIN (i2, 64)));
               r > 0 && i < width;
               d++, r--, i1 += 4, i2 -= 4, i++)
            {
              d[0] = last;
              last += rand() % 16 - 4;
              d[width] = last;
              last += rand() % 16 - 4;
              d[2 * width] = last;
              last += rand() % 16 - 4;
            }
        }
    }
  else if (++fire->frames >= 2 * height)
    {
      b_module_request_stop (module);
      return 0;
    }

  for (i = width * height, d = module->buffer; i > width * 2 + 1; d++, i--)
    *d = fire->table[(d[width - 1] + d[width + 1] + d[width] +
                      d[2 * width - 1] + d[2 * width + 1])];

  for  (; i > width + 1; d++, i--)
    *d = fire->table[(d[width - 1] + d[width + 1] + d[width] +
                      2 * d[0])];

  for  (; i > 1; d++, i--)
    *d = fire->table[d[- 1] + 3 * d[0] + d[1]];

  *d = fire->table[2 * d[- 1] + 3 * d[0]];
 
  b_module_paint (module);

  return TIMEOUT;
}

static void
b_fire_describe (BModule      *module,
                 const gchar **title,
                 const gchar **description,
                 const gchar **author)
{
  *title       = "BFire";
  *description = "Fire hack";
  *author      = "Sven Neumann";
}

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

#include <glib.h>
#include <gmodule.h>

#include <string.h>

#include <blib/blib.h>

#define TIMEOUT 5

#define B_TYPE_DROPOUT         (b_type_dropout)
#define B_DROPOUT(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_DROPOUT, BDropout))
#define B_DROPOUT_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_DROPOUT, BDropoutClass))
#define B_IS_DROPOUT(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_DROPOUT))

typedef struct _BDropout      BDropout;
typedef struct _BDropoutClass BDropoutClass;

struct _BDropout
{
  BModule   parent_instance;

  gboolean *dropmap;
  gint      n;
};

struct _BDropoutClass
{
  BModuleClass  parent_class;
};

static GType      b_dropout_get_type     (GTypeModule  *module);

static void       b_dropout_class_init   (BDropoutClass   *klass);
static void       b_dropout_init         (BDropout        *dropout);

static gboolean   b_dropout_query        (gint          width,
					  gint          height,
					  gint          channels,
					  gint          maxval);
static gboolean   b_dropout_prepare      (BModule      *module,
					  GError      **error);
static void       b_dropout_relax        (BModule      *module);
static void       b_dropout_start        (BModule      *module);
static gint       b_dropout_tick         (BModule      *module);
static void       b_dropout_describe     (BModule      *module,
					  const gchar **title,
					  const gchar **description,
					  const gchar **author);


static GType  b_type_dropout = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_dropout_get_type (module);
  return TRUE;
}

GType
b_dropout_get_type (GTypeModule *module)
{
  if (!b_type_dropout)
    {
      static const GTypeInfo dropout_info =
	{
	  sizeof (BDropoutClass),
	  NULL,           /* base_init */
	  NULL,           /* base_finalize */
	  (GClassInitFunc) b_dropout_class_init,
	  NULL,           /* class_finalize */
	  NULL,           /* class_data */
	  sizeof (BDropout),
	  0,              /* n_preallocs */
	  (GInstanceInitFunc) b_dropout_init,
	};
     
      b_type_dropout = g_type_module_register_type (module,
						    B_TYPE_MODULE, "BDropout",
						    &dropout_info, 0);
    }
 
  return b_type_dropout;
}

static void
b_dropout_class_init (BDropoutClass *klass)
{
  GObjectClass *object_class;
  BModuleClass *module_class;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);
 
  module_class->query    = b_dropout_query;
  module_class->prepare  = b_dropout_prepare;
  module_class->relax    = b_dropout_relax;
  module_class->start    = b_dropout_start;
  module_class->tick     = b_dropout_tick;
  module_class->describe = b_dropout_describe;
}

static void
b_dropout_init (BDropout *dropout)
{
}

static gboolean
b_dropout_query (gint     width,
		 gint     height,
		 gint     channels,
		 gint     maxval)
{
  return (width > 0 && height > 3 && channels == 1 && maxval == 255);
}

static gboolean
b_dropout_prepare (BModule  *module,
		   GError  **error)
{
  BDropout *dropout = B_DROPOUT (module);
  /*  gint x, y; */

  g_return_val_if_fail (module->height > 0, FALSE);

  /* HACK */
  /*
  for (x=0; x<module->width; x++)
    for (y=0; y<module->height; y++)
      b_module_draw_point (module, x, y, 0xff);
  */

  dropout->dropmap = g_new0 (gboolean, module->width * module->height);
  dropout->n = 0;
 
  return TRUE;
}

static void
b_dropout_relax (BModule *module)
{
  BDropout *dropout = B_DROPOUT (module);
 
  if (dropout->dropmap)
    {
      g_free (dropout->dropmap);
      dropout->dropmap = NULL;
    }
}

static void
b_dropout_start (BModule *module)
{
  b_module_ticker_start (module, b_dropout_tick (module));
}

static gint
b_dropout_tick (BModule *module)
{
  BDropout  *dropout = B_DROPOUT (module);
  gint       x, y, n;

  for (n = 0; n < 5; n++)
    {
      if (++dropout->n == module->width * module->height)
	{
	  b_module_fill (module, 0);
	  b_module_paint (module);
	  b_module_request_stop (module);
	  return 0;
	}
     
      do
	{
	  x = random () % module->width;
	  y = random () % module->height;
	}
      while (dropout->dropmap[(y * module->width) + x]);
     
      dropout->dropmap[(y * module->width) + x] = TRUE;
      b_module_draw_point (module, x, y, 0);
    }
 
  b_module_paint (module);
 
  return TIMEOUT;
}

static void
b_dropout_describe (BModule      *module,
                 const gchar **title,
                 const gchar **description,
                 const gchar **author)
{
  *title       = "BDropout";
  *description = "Dropout hack";
  *author      = "Daniel Mack";
}

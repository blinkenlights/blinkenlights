/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2006  The Blinkenlights Crew
 *                     Stefan Schuermans <1stein@blinkenarea.org>
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

#include <math.h>
#include <string.h>

#include <blib/blib.h>

#define TIMEOUT  100
#define MAX_FIREWORKS 16
#define MIN_OBJECTS 3
#define MAX_OBJECTS 8
#define NEW_FIREWORKS_STEPS 30 /* 1/probability that an inactive firework is created in a step */
#define RISE_STEP 0.07
#define SPREAD_STEP 0.03
#define FADE16 25 /* 16 for no fading, greater for more fading */

#define B_TYPE_FIREWORKS         (b_type_fireworks)
#define B_FIREWORKS(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_FIREWORKS, BFireworks))
#define B_FIREWORKS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_FIREWORKS, BFireworksClass))
#define B_IS_FIREWORKS(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_FIREWORKS))

typedef struct _tFirework {
  gint active; /* if firework is active */
  gint center_x, center_y; /* center of firework */
  gint max_radius; /* maximum radius to reach */
  gint num_objects; /* number of objects */
  double base_angle; /* angle of first object */
  float rise; /* 0..1, increased during rise phase */
  float spread; /* 0..1, increased during explosion phase */
} tFirework;

typedef struct _BFireworks BFireworks;
typedef struct _BFireworksClass BFireworksClass;

struct _BFireworks
{
  BModule parent_instance;

  guchar * data;
  guchar * data2;

  guint n_fireworks;
  tFirework fireworks[MAX_FIREWORKS];
};

struct _BFireworksClass
{
  BModuleClass parent_class;
};


static GType b_fireworks_get_type (GTypeModule * module);

static void b_fireworks_class_init (BFireworksClass * klass);
static void b_fireworks_init (BFireworks * fireworks);

static gboolean b_fireworks_query (gint width,
				   gint height, gint channels, gint maxval);
static gboolean b_fireworks_prepare (BModule * module, GError ** error);
static void b_fireworks_start (BModule * module);
static void b_fireworks_relax (BModule * module);
static gint b_fireworks_tick (BModule * module);
static void b_fireworks_describe (BModule * module,
				  const gchar ** title,
				  const gchar ** description,
				  const gchar ** author);


static GType b_type_fireworks = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule * module)
{
  b_fireworks_get_type (module);
  return TRUE;
}

GType
b_fireworks_get_type (GTypeModule * module)
{
  if (!b_type_fireworks)
    {
      static const GTypeInfo fireworks_info = {
	sizeof (BFireworksClass),
	NULL,			/* base_init */
	NULL,			/* base_finalize */
	(GClassInitFunc) b_fireworks_class_init,
	NULL,			/* class_finalize */
	NULL,			/* class_data */
	sizeof (BFireworks),
	0,			/* n_preallocs */
	(GInstanceInitFunc) b_fireworks_init,
      };

      b_type_fireworks = g_type_module_register_type (module,
						      B_TYPE_MODULE,
						      "BFireworks",
						      &fireworks_info, 0);
    }

  return b_type_fireworks;
}

static void
b_fireworks_class_init (BFireworksClass * klass)
{
  GObjectClass *object_class;
  BModuleClass *module_class;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  module_class->query = b_fireworks_query;
  module_class->prepare = b_fireworks_prepare;
  module_class->relax = b_fireworks_relax;
  module_class->start = b_fireworks_start;
  module_class->tick = b_fireworks_tick;
  module_class->describe = b_fireworks_describe;
}

static void
b_fireworks_init (BFireworks * fireworks)
{
}

static gboolean
b_fireworks_query (gint width, gint height, gint channels, gint maxval)
{
  return (width > 3 && height > 3 && channels == 1 && maxval == 255);
}

static gboolean
b_fireworks_prepare (BModule * module, GError ** error)
{
  BFireworks *fireworks = B_FIREWORKS (module);
  gint width = module->width;
  gint height = module->height;

  fireworks->data = (guchar *)g_malloc (width * height);
  memset (fireworks->data, 0, width * height);
  fireworks->data2 = (guchar *)g_malloc (width * height);
  memset (fireworks->data2, 0, width * height);

  fireworks->n_fireworks = MAX (width / height, height / width);
  if (fireworks->n_fireworks > MAX_FIREWORKS)
    fireworks->n_fireworks = MAX_FIREWORKS;

  return TRUE;
}

static void
b_fireworks_relax (BModule * module)
{
  BFireworks *fireworks = B_FIREWORKS (module);

  g_free (fireworks->data);
  fireworks->data = NULL;
  g_free (fireworks->data2);
  fireworks->data2 = NULL;
}

static void
b_fireworks_start (BModule * module)
{
  BFireworks *fireworks = B_FIREWORKS (module);
  gint i;

  for (i = 0; i < fireworks->n_fireworks; i++)
    fireworks->fireworks[i].active = 0;

  b_module_ticker_start (module, b_fireworks_tick (module));
}

static gint
b_fireworks_tick (BModule * module)
{
  BFireworks *fireworks = B_FIREWORKS (module);
  gint width = module->width;
  gint height = module->height;
  float aspect = module->aspect;
  gint maxval = module->maxval;
  float base_radius = MIN ((float)height, (float)width * aspect)  * 0.3;
  gint min_radius = (gint)(base_radius);
  gint max_radius = (gint)(base_radius * 2);
  guchar * data = fireworks->data;
  guchar * data2 = fireworks->data2;
  gint x, y, i;

  /* create new fireworks */
  for (i = 0; i < fireworks->n_fireworks; i++)  {
    tFirework * fw = &fireworks->fireworks[i];
    if (! fw->active && rand () < RAND_MAX / NEW_FIREWORKS_STEPS)  {
      fw->active = 1;
      fw->center_x = rand () % width;
      fw->center_y = rand () % height;
      fw->max_radius = min_radius + rand () % (max_radius - min_radius + 1);
      fw->num_objects = MIN_OBJECTS + rand () % (MAX_OBJECTS - MIN_OBJECTS + 1);
      fw->base_angle = 6.28 * (double)rand () / (double)RAND_MAX;
      fw->rise = 0.0;
      fw->spread = 0.0;
    }
  }
  /* blur and fade out */
  memcpy (data2, data, width * height);
  for (y = 0, i = 0; y < height; y++)  {
    for (x = 0; x < width; x++, i++)  {
      gint l = x > 0 ? x - 1 : 0, r = x < width - 1 ? x + 1 : width - 1;
      gint t = y > 0 ? y - 1 : 0, b = y < height - 1 ? y + 1 : height - 1;
      gint c = 1 * ((gint)data2[t * width + l] + (gint)data2[t * width + r]
                  + (gint)data2[b * width + l] + (gint)data2[b * width + r])
             + 2 * ((gint)data2[y * width + l] + (gint)data2[y * width + r]
                  + (gint)data2[t * width + x] + (gint)data2[b * width + x])
             + 4 * (gint)data2[y * width + x];
      data[i] = (guchar)(c / FADE16);
    }
  }

  /* draw fireworks */
  for (i = 0; i < fireworks->n_fireworks; i++)  {
    tFirework * fw = &fireworks->fireworks[i];
    if (fw->active)  {
      /* rise phase */
      if (fw->rise < 1.0)  {
        guchar c;
        fw->rise += RISE_STEP;
        c = (guchar)(fw->rise * (float)0xFF + 0.5);
        x = fw->center_x;
        y = fw->center_y;
        if (x >= 0 && x < width && y >= 0 && y < height)  {
          guchar * ptr = &data[y * width + x];
          *ptr = MAX (*ptr, c);
        }
      /* spread phase */
      } else if (fw->spread < 1.0)  {
        guchar c;
        gint j;
        fw->spread += SPREAD_STEP;
        c = (guchar)((1.0 - 0.5 * fw->spread) * (float)module->maxval + 0.5);
        for (j = 0; j < fw->num_objects; j++)  {
          double radius = fw->spread * fw->max_radius;
          double angle = fw->base_angle + 6.28 * (double)j / (double)fw->num_objects;
          x = (gint)((double)fw->center_x + radius / (double)aspect * (double)cos (angle)  + 0.5);
          y = (gint)((double)fw->center_y + radius * (double)sin (angle)  + 0.5);
          if (x >= 0 && x < width && y >= 0 && y < height)  {
            guchar * ptr = &data[y * width + x];
            *ptr = MAX (*ptr, c);
          }
        }
      /* done */
      } else
        fw->active = 0;
    }
  }

  /* draw data to display */
  for (y = 0, i = 0; y < height; y++)  {
    for (x = 0; x < width; x++, i++)  {
      b_module_draw_point (module, x, y, (gint)data[i] * maxval / 0xFF);
    }
  }
  b_module_paint (module);

  return TIMEOUT;
}

static void
b_fireworks_describe (BModule * module,
		      const gchar ** title,
		      const gchar ** description, const gchar ** author)
{
  *title = "BFireworks";
  *description = "Fireworks hack";
  *author = "Stefan Schuermans";
}

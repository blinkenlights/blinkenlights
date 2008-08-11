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

#include "config.h"

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

#include <glib.h>
#include <gmodule.h>

#include <blib/blib.h>

#include "digits.h"


#define TIMEOUT       75
#define DEFAULT_MODE  CLOCK_MODE_DIGITAL


typedef enum
{
  CLOCK_MODE_DIGITAL,
  CLOCK_MODE_ANALOG,
  CLOCK_MODE_LAST
} BClockMode;

enum
{
  PROP_0,
  PROP_MODE
};


#define B_TYPE_CLOCK_MODE    (b_clock_mode_get_type ())

#define B_TYPE_CLOCK         (b_type_clock)
#define B_CLOCK(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_CLOCK, BClock))
#define B_CLOCK_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_CLOCK, BClockClass))
#define B_IS_CLOCK(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_CLOCK))

typedef struct _BClock      BClock;
typedef struct _BClockClass BClockClass;

struct _BClock
{
  BModule      parent_instance;

  gint         with_secs;
  gint         num_digits;
  const BFont  *digits;

  guint        epoche;
  gint         mode;
  gboolean     colon;
};

struct _BClockClass
{
  BModuleClass  parent_class;
};

static GType      b_clock_get_type     (GTypeModule   *module);

static void       b_clock_class_init   (BClockClass   *klass);
static void       b_clock_set_property (GObject       *object,
					guint          property_id,
					const GValue  *value,
					GParamSpec    *pspec);
static gboolean   b_clock_query        (gint           width,
					gint           height,
					gint           channels,
					gint           maxval);
static gboolean   b_clock_prepare      (BModule       *module,
					GError       **error);
static void       b_clock_relax        (BModule       *module);
static void       b_clock_start        (BModule       *module);
static gint       b_clock_tick         (BModule       *module);
static void       b_clock_describe     (BModule       *module,
					const gchar  **title,
					const gchar  **description,
					const gchar  **author);
static gboolean   b_clock_gettime      (BClock        *clock);


static GType  b_type_clock = 0;


static const GEnumValue clock_mode_enum_values[] =
{
  { CLOCK_MODE_DIGITAL, "digital", NULL },
  { CLOCK_MODE_ANALOG,  "analog",  NULL },
  { 0, NULL, NULL }
};

GType
b_clock_mode_get_type (void)
{
  static GType enum_type = 0;

  if (!enum_type)
    enum_type = g_enum_register_static ("BClockMode",
                                        clock_mode_enum_values);

  return enum_type;
}

G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_clock_get_type (module);
  return TRUE;
}

GType
b_clock_get_type (GTypeModule *module)
{
  if (!b_type_clock)
    {
      static const GTypeInfo clock_info =
	{
	  sizeof (BClockClass),
	  NULL,           /* base_init      */
	  NULL,           /* base_finalize  */
	  (GClassInitFunc) b_clock_class_init,
	  NULL,           /* class_finalize */
	  NULL,           /* class_data     */
	  sizeof (BClock),
	  0,              /* n_preallocs    */
	  NULL            /* instance_init  */
	};

      b_type_clock = g_type_module_register_type (module,
						  B_TYPE_MODULE, "BClock",
						  &clock_info, 0);
    }

  return b_type_clock;
}

static void
b_clock_class_init (BClockClass *klass)
{
  GObjectClass *object_class;
  BModuleClass *module_class;
  GParamSpec   *param_spec;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  object_class->set_property = b_clock_set_property;

  param_spec = g_param_spec_enum ("mode", NULL,
				  "Clock mode (analog/digital)",
                                  B_TYPE_CLOCK_MODE, DEFAULT_MODE,
                                  G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_MODE, param_spec);

  module_class->query    = b_clock_query;
  module_class->prepare  = b_clock_prepare;
  module_class->relax    = b_clock_relax;
  module_class->start    = b_clock_start;
  module_class->tick     = b_clock_tick;
  module_class->describe = b_clock_describe;
}

static void
b_clock_set_property (GObject      *object,
		      guint         property_id,
		      const GValue *value,
		      GParamSpec   *pspec)
{
  BClock *line = B_CLOCK (object);

  switch (property_id)
    {
    case PROP_MODE:
      line->mode = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
b_clock_query (gint width,
	       gint height,
	       gint channels,
	       gint maxval)
{
  return (width > 0 && height > 3 && channels == 1 && maxval == 255);
}

static gboolean
b_clock_prepare (BModule  *module,
		 GError  **error)
{
  return TRUE;
}

static void
b_clock_relax (BModule *module)
{
}

static void
b_clock_start (BModule *module)
{
  BClock * clock = (BClock *)module;

  gint f;
  const BFont *fonts[] = { &b_digits_3x5,
                           &b_digits_5x7,
                           &b_digits_8x7,
                           &b_digits_8x7wide };

  clock->with_secs = module->width >= MIN (25, 2 * module->height);
  clock->num_digits = clock->with_secs ? 8 : 5;

  for (f = (sizeof (fonts) / sizeof (fonts[0])) - 1;
       module->width - ((clock->num_digits - 1) * fonts[f]->advance + fonts[f]->width) < 0
       || module->width - fonts[f]->height < 0;
       f--)
    {
      if (f < 0)
        {
          b_module_request_stop (module);
          return;
        }
    }
  clock->digits = fonts[f];

  b_clock_gettime (clock);
  b_module_ticker_start (module, b_clock_tick (module));
}

static gboolean
b_clock_gettime (BClock *clock)
{
  struct timeval tv;

  gettimeofday (&tv, NULL);

  if (tv.tv_sec != clock->epoche)
    {
      clock->epoche = tv.tv_sec;
      return TRUE;
    }

  return FALSE;
}

static gint
b_clock_tick (BModule *module)
{
  gint h, m, s;
  BClock *clock = B_CLOCK (module);
  struct tm *tm;

  if (!b_clock_gettime (clock))
    return TIMEOUT;

  b_module_fill (module, 0);

  tm = localtime ((time_t *) &clock->epoche);
  h = tm->tm_hour;
  m = tm->tm_min;
  s = tm->tm_sec;

  /*  g_print ("%02d:%02d:%02d\n", h, m, s);  */

  switch (clock->mode)
    {
    case CLOCK_MODE_DIGITAL:
      {
	gchar *text;
	gint   len, i, n, x, y, x0, y0;

        if (clock->with_secs)
	  {
            text = g_strdup_printf ("%02d:%02d:%02d", h, m, s);
          }
        else
          {
            text = g_strdup_printf (clock->colon ? "%02d:%02d" : "%02d %02d",
                                    h, m);
            clock->colon = !clock->colon;
          }

	len = strlen (text);

        x0 = (module->width - (clock->num_digits - 1) * clock->digits->advance - clock->digits->width) / 2;
	y0 = (module->height - clock->digits->height) / 2;

	for (n = 0; n < len; n++)
	  {
	    for (i = 0; i < clock->digits->num_digits && clock->digits->digits_str[i] != text[n]; i++);

	    if (len * clock->digits->advance > module->width)
              {
              if (text[n-1] == ':' || text[n-1] == ' ')
                  x0--;

                if (text[n] == ':' || text[n] == ' ')
                  x0--;
              }

	    if (i < clock->digits->num_digits)
	      for (x = 0; x < clock->digits->width; x++)
		for (y = 0; y < clock->digits->height; y++)
		  if (clock->digits->data[i][y * clock->digits->width + x] != '0')
		    b_module_draw_point (module,
                                         x0 + x, y0 + y, module->maxval);

	    x0 += clock->digits->advance;
	  }
	g_free (text);
      }

      break;
    case CLOCK_MODE_ANALOG:
      {
	gint x0, y0, r, x, y, a;

	x0 = module->width  / 2;
	y0 = module->height / 2;
	r  = MIN (x0, y0 / module->aspect);

	for (a = 0; a < 360; a += 30)
	  {
	    x = cos (a/180.0 * G_PI) * r + x0 + 0.5;
	    y = (sin (a/180.0 * G_PI) * r) * module->aspect + y0 + 0.5;

	    b_module_draw_point (module, x, y, 0x7f);
	  }

	/* 1st hand */
	a = h;
	a += 45;
	a *= 30;
	x = cos (a/180.0 * G_PI) * (r - 4) + x0 + 0.5;
	y = (sin (a/180.0 * G_PI) * (r - 4)) * module->aspect + y0 + 0.5;
	b_module_draw_line (module, x0, y0, x, y, 0x80);

	/* 2nd hand */
	a = m;
	a += 45;
	a *= 6;
	x = cos (a/180.0 * G_PI) * (r - 2) + x0 + 0.5;
	y = (sin (a/180.0 * G_PI) * (r - 2)) * module->aspect + y0 + 0.5;
	b_module_draw_line (module, x0, y0, x, y, 0xe0);


	/* 3rd hand */
	a = s;
	a += 9;
	a *= 6;
	x = cos (a/180.0 * G_PI) * (r - 1) + x0 + 0.5;
	y = (sin (a/180.0 * G_PI) * (r - 1)) * module->aspect + y0 + 0.5;
	b_module_draw_line (module, x0, y0, x, y, 0xff);
      }
      break;
    }

  b_module_paint (module);
  return TIMEOUT;
}

static void
b_clock_describe (BModule      *module,
		  const gchar **title,
		  const gchar **description,
		  const gchar **author)
{
  *title       = "BClock";
  *description = "BKloppte BClock";
  *author      = "Daniel Mack";
}

/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2002  The Blinkenlights Crew
 *                     Sven Neumann <sven@gimp.org>
 *                     Daniel Mack <daniel@yoobay.net>
 *
 * Inspired by the great 2001 sylvester countdown on the HDL.
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
#include <sys/time.h>
#include <unistd.h>

#include <glib.h>
#include <gmodule.h>

#include <blib/blib.h>

#include "digits.h"

#define B_TYPE_COUNTDOWN         (b_type_countdown)
#define B_COUNTDOWN(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_COUNTDOWN, BCountdown))
#define B_COUNTDOWN_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_COUNTDOWN, BCountdownClass))
#define B_IS_COUNTDOWN(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_COUNTDOWN))

#define MAX_DIGITS 8

typedef struct _BCountdown      BCountdown;
typedef struct _BCountdownClass BCountdownClass;

struct _BCountdown
{
  BModule   parent_instance;

  gchar last_digits[MAX_DIGITS];
  gchar last_font;
  gchar last_number_of_digits;

  gint      end;
  gint      duration;
};

struct _BCountdownClass
{
  BModuleClass  parent_class;
};

enum
{
  PROP_0,
  PROP_END,
  PROP_DURATION
};


static GType    b_countdown_get_type     (GTypeModule     *module);
static void     b_countdown_class_init   (BCountdownClass *klass);
static void     b_countdown_set_property (GObject         *object,
					  guint            property_id,
					  const GValue    *value,
					  GParamSpec      *pspec);
static gboolean b_countdown_query        (gint             width,
					  gint             height,
					  gint             channels,
					  gint             maxval);
static gboolean b_countdown_prepare      (BModule         *module,
					  GError         **error);
static void     b_countdown_start        (BModule         *module);
static gint     b_countdown_tick         (BModule         *module);
static void     b_countdown_describe     (BModule         *module,
					  const gchar    **title,
					  const gchar    **description,
					  const gchar    **author);


static BModuleClass * parent_class     = NULL;
static GType          b_type_countdown = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_countdown_get_type (module);
  return TRUE;
}

GType
b_countdown_get_type (GTypeModule *module)
{
  if (!b_type_countdown)
    {
      static const GTypeInfo countdown_info =
      {
        sizeof (BCountdownClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_countdown_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BCountdown),
        0,              /* n_preallocs */
        NULL            /* instance_init */
      };

      b_type_countdown = g_type_module_register_type (module,
						      B_TYPE_MODULE,
						      "BCountdown",
						      &countdown_info, 0);
    }

  return b_type_countdown;
}

static void
b_countdown_class_init (BCountdownClass *klass)
{
  GObjectClass *object_class;
  BModuleClass *module_class;
  GParamSpec   *param_spec;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->set_property = b_countdown_set_property;

  param_spec = g_param_spec_int ("end", NULL,
				 "When to end the countdown.",
				 0, G_MAXINT, 0,
				 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_END, param_spec);

  param_spec = g_param_spec_int ("duration", NULL,
				 "How long to run the countdown.",
				 3, 999999, 999,
				 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_DURATION, param_spec);

  module_class->query    = b_countdown_query;
  module_class->prepare  = b_countdown_prepare;
  module_class->start    = b_countdown_start;
  module_class->tick     = b_countdown_tick;
  module_class->describe = b_countdown_describe;
}

static void
b_countdown_set_property (GObject      *object,
                    guint         property_id,
                    const GValue *value,
                    GParamSpec   *pspec)
{
  BCountdown *countdown = B_COUNTDOWN (object);

  switch (property_id)
    {
    case PROP_END:
      countdown->end = g_value_get_int (value);
      break;

    case PROP_DURATION:
      countdown->duration = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
b_countdown_query (gint  width,
		   gint  height,
		   gint  channels,
		   gint  maxval)
{
  return (width >= 12 && height >= 7 && channels == 1 && maxval > 0);
}

static gboolean
b_countdown_prepare (BModule  *module,
		     GError  **error)
{
  BCountdown *countdown = B_COUNTDOWN (module);

  if (countdown->end < 0)
    {
      g_set_error (error, 0, 0, "Invalid end time for countdown.");
      return FALSE;
    }

  return TRUE;
}

static void
b_countdown_start (BModule *module)
{
  gint x;
 
  BCountdown     *countdown = B_COUNTDOWN (module);
  countdown->last_font = -1;
  countdown->last_number_of_digits = -1;

  for (x = 0; x < MAX_DIGITS; x++)
    {
      countdown->last_digits[x] = -1;
    } 
 
  b_module_ticker_start (module, 1);
}

static gint
b_countdown_tick (BModule *module)
{
  BCountdown     *countdown = B_COUNTDOWN (module);
  struct timeval  tv;
  gint            x0, y0;
  gint            x, y;
  gint            n, d, i, f;
  gint            scrollstep;
  gint            scroll[MAX_DIGITS];
  gint            new_digits[MAX_DIGITS];
  const BFont    *fonts[] = { &b_digits_3x5,
                              &b_digits_5x7,
                              &b_digits_8x7,
                              &b_digits_8x7wide };

  gettimeofday (&tv, NULL);

  n = countdown->end - tv.tv_sec;

  g_print ("BCountdown: %d\n", n);

  if (n <= 0 || n > countdown->duration)
    {
      b_module_request_stop (module);
      return -1;
    }

  for (d = 1, i = n; i > 9; d++)
    i /= 10;
 
  for (f = (sizeof (fonts) / sizeof (fonts[0])) - 1;
       module->width - ((d - 1) * fonts[f]->advance + fonts[f]->width) < 0
       || module->width - fonts[f]->height < 0;
       f--)
    {
      if (f < 0)
        {
          b_module_request_stop (module);
          return -1;
        }
    }

  for (i = 0; i < MAX_DIGITS; i++)
    new_digits[i] = ' ';
 
  g_print ("using font #%d, w: %d h: %d a: %d\n",
           f, fonts[f]->width, fonts[f]->height, fonts[f]->advance);

  if (countdown->last_font != f ||
      countdown->last_number_of_digits != d)
    b_module_fill (module, 0);

  for (i = d; i > 0; i--)
    {
      new_digits[i] = n % 10;
      scroll[i] = (countdown->last_digits[i] == -1)
                || (countdown->last_digits[i] != new_digits[i]);
      n /= 10;
    }

  for (scrollstep = 0; scrollstep < module->height; scrollstep++)
    {
      x0 = (module->width + (d - 1) * fonts[f]->advance - fonts[f]->width) / 2;
      y0 = (module->height - fonts[f]->height) / 2;

      for (i = d; i > 0; i--)
        {
          if (scroll[i])
            {
              for (x = 0; x < fonts[f]->width; x++)
                {
                  for (y = module->height; y > 0; y--)
                    {
                      module->buffer[(y-1) * module->width + x + x0] =
                        module->buffer[(y-2) * module->width + x + x0]?
                        (module->maxval/2):0;
                    }
                  module->buffer[x + x0] = 0;
                 
                  for (y = 0; y < fonts[f]->height; y++)
                    {
                      if (y + y0 <= scrollstep + 1)
                        b_module_draw_point (module, x0 + x, y0 + y + (scrollstep - module->height + 1),
                                             (fonts[f]->data[new_digits[i]][y * fonts[f]->width + x] != '0')?
                                             module->maxval:0);
                    }
                }
            }
          x0 -= fonts[f]->advance;
        }
      b_module_paint (module);
      usleep (33 * 1000);
    }

  for (x = 0; x < MAX_DIGITS; x++)
    {
      countdown->last_digits[x] = new_digits[x];
    }
 
  b_module_paint (module);

  countdown->last_font = f;
  countdown->last_number_of_digits = d;
 
  gettimeofday (&tv, NULL);
  return 1000 - (tv.tv_usec / 1000);

}

static void
b_countdown_describe (BModule      *module,
		      const gchar **title,
		      const gchar **description,
		      const gchar **author)
{
  *title       = "BCountdown";
  *description = "Countdown";
  *author      = "Sven Neumann";
}

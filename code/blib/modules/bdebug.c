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

/* Test implementation for a BModule as used in the Blinkenlights context.
 * BModules are used to implement games, debug tools etc.
 * You can basically use this skeleton, rename the functions and hack your
 * code into the functions that contain all the g_print()s now.
 */

#include <string.h>

#include <glib-object.h>
#include <gmodule.h>

#include <blib/blib.h>

#include "digits.h"


#define B_TYPE_DEBUG         (b_type_debug)
#define B_DEBUG(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_DEBUG, BDebug))
#define B_DEBUG_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_DEBUG, BDebugClass))
#define B_IS_DEBUG(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_DEBUG))

typedef struct _BDebug      BDebug;
typedef struct _BDebugClass BDebugClass;

struct _BDebug
{
  BModule   parent_instance;

  gint      x, y, val;
  gboolean  dimming;
  gboolean  runmode;
};

struct _BDebugClass
{
  BModuleClass  parent_class;
};

static GType     b_debug_get_type   (GTypeModule  *module);
static void      b_debug_class_init (BDebugClass  *klass);
static void      b_debug_init       (BDebug       *debug);
static void      b_debug_finalize   (GObject      *object);
static gboolean  b_debug_query      (gint          width,
				     gint          height,
				     gint          channels,
				     gint          maxval);
static gboolean  b_debug_prepare    (BModule      *module,
				     GError      **error);
static void      b_debug_start      (BModule      *module);
static void      b_debug_stop       (BModule      *module);
static void      b_debug_event      (BModule      *module,
				     BModuleEvent *event);
static gint      b_debug_tick       (BModule      *module);
static void      b_debug_describe   (BModule      *module,
				     const gchar **title,
				     const gchar **description,
				     const gchar **author);


static BModuleClass * parent_class = NULL;
static GType          b_type_debug = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_debug_get_type (module);
 
  return TRUE;
}

GType
b_debug_get_type (GTypeModule *module)
{
  if (! b_type_debug)
    {
      static const GTypeInfo debug_info =
      {
        sizeof (BDebugClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_debug_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BDebug),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_debug_init,
      };

      b_type_debug = g_type_module_register_type (module,
						  B_TYPE_MODULE,
						  "BDebug",
						  &debug_info,
						  0);
    }

  return b_type_debug;
}

static void
b_debug_class_init (BDebugClass *klass)
{
  GObjectClass *object_class;
  BModuleClass *module_class;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = b_debug_finalize;

  module_class->query    = b_debug_query;
  module_class->prepare  = b_debug_prepare;
  module_class->start    = b_debug_start;
  module_class->stop     = b_debug_stop;
  module_class->event    = b_debug_event;
  module_class->tick     = b_debug_tick;
  module_class->describe = b_debug_describe;
}


static void
paint_digits (BDebug *debug)
{
  gint     x, y, y_off, x_off = 3, n;
  gint     foo[4];
  BModule *module = B_MODULE (debug);
  BFont    digits = b_digits_3x5;

  g_print ("BDebug: %02d %02d\n", debug->x, debug->y);

  if (module->width < 12 || module->height < 12)
    return;

  if (debug->y < 9)
    y_off = debug->y + 4;
  else
    y_off = debug->y - 8;

  foo[0] = ((module->height - debug->y) / 10);
  foo[1] = ((module->height - debug->y) % 10);
  foo[2] = ((debug->x + 1) / 10);
  foo[3] = ((debug->x + 1) % 10);

  for (n = 0; n < 4; n++)
    {
      for (x = 0; x < digits.width; x++)
	for (y = 0; y < digits.height; y++)
          if (digits.data[foo[n]][y*digits.width + x] != '0')
            b_module_draw_point (module, x + x_off, y + y_off, module->maxval);

      if (n == 1)
	x_off += digits.advance;

      x_off += x+1;
    }
}

static void
b_debug_init (BDebug *debug)
{
}

static void
b_debug_finalize (GObject *object)
{
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
b_debug_query (gint     width,
	       gint     height,
	       gint     channels,
	       gint     maxval)
{
  return (width > 0 && height > 0 && channels == 1 && maxval > 0);
}

static gboolean
b_debug_prepare (BModule  *module,
		 GError  **error)
{
  BDebug *debug = B_DEBUG (module);

  debug->x       = module->width  / 2;
  debug->y       = module->height / 2;
  debug->val     = module->maxval;
  debug->dimming = FALSE;
  debug->runmode = FALSE;

  return TRUE;
}

static void
b_debug_start (BModule *module)
{
  BDebug *debug = B_DEBUG (module);

  b_module_draw_point (module, debug->x, debug->y, debug->val);
  b_module_paint (module);
}

static void
b_debug_stop (BModule *module)
{
  /* clear the screen */
  b_module_fill (module, 0);
  b_module_paint (module);
}

static void
b_debug_event (BModule      *module,
	       BModuleEvent *event)
{
  BDebug *debug = B_DEBUG (module);

  switch (event->device_id)
    {
    case 0:
      switch (event->type)
        {
        case B_EVENT_TYPE_KEY:
          switch (event->key)
            {
            case B_KEY_1:
	      debug->x--;	
            case B_KEY_2:
	      debug->y--;	
              break;

            case B_KEY_7:
	      debug->y++;
            case B_KEY_4:
	      debug->x--;	
              break;

            case B_KEY_3:
	      debug->y--;
            case B_KEY_6:
	      debug->x++;
              break;

            case B_KEY_9:
	      debug->x++;
            case B_KEY_8:
	      debug->y++;
              break;

            case B_KEY_0:
	      if (debug->runmode)
		break;

	      if (debug->dimming)
		{
		  debug->dimming = FALSE;
		  b_module_ticker_stop (module);
		}
	      else
		{
		  debug->dimming = TRUE;
		  b_module_ticker_start (module, 1000);
		}
	      break;
		 
	    case B_KEY_5:
	      debug->val += module->maxval/8;
	      if (debug->val > module->maxval)
		debug->val = 0;
	      break;

	    case B_KEY_HASH:
	      if (debug->dimming)
		{
		  debug->dimming = FALSE;
		  b_module_ticker_stop (module);
		}

	      debug->val = module->maxval;

	      if (debug->runmode)
		{
		  debug->runmode = FALSE;
		  b_module_ticker_stop (module);
		}
	      else
		{
		  debug->runmode = TRUE;
		  b_module_ticker_start (module, 250);
		}
	      break;
	    
	    case B_KEY_ASTERISK:
	      debug->val = module->maxval;
	      break;

            default:
              break;
            }
          break;
        default:
          break;
        }
    case 1:
      break;
    }

  if (debug->x > module->width - 1)
    {
      debug->x = 0;
      debug->y++;
    }

  if (debug->y > module->height - 1)
    debug->y = 0;

  if (debug->x < 0)
    {
      debug->x = module->width - 1;
      debug->y--;
    }

  if (debug->y < 0)
    debug->y = module->height - 1;

  b_module_fill (module, 0);
  b_module_draw_point (module, debug->x, debug->y, debug->val);

  paint_digits (debug);

  b_module_paint (module);
}

static gint
b_debug_tick (BModule *module)
{
  BDebug       *debug = B_DEBUG (module);
  BModuleEvent  ev;

  ev.device_id = 0;
  ev.type      = B_EVENT_TYPE_KEY;

  if (debug->dimming)
    ev.key = B_KEY_5;
  else if (debug->runmode)
    ev.key = B_KEY_6;

  b_debug_event (module, &ev);

  if (debug->dimming)
    return 1000;
  else if (debug->runmode)
    return 333;

  return 0;
}

static void
b_debug_describe (BModule      *module,
		  const gchar **title,
		  const gchar **description,
		  const gchar **author)
{
  *title       = "BDebug";
  *description = "Blinkenlights Debug swiss army knife";
  *author      = "Blinkenlights Crew";
}

/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2002  The Blinkenlights Crew
 *                     Daniel Mack <daniel@yoobay.net>
 *                     Michael Natterer <mitch@gimp.org>
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


#define B_TYPE_TEST_MODULE         (b_type_test_module)
#define B_TEST_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_TEST_MODULE, BTestModule))
#define B_TEST_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_TEST_MODULE, BTestModuleClass))
#define B_IS_TEST_MODULE(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_TEST_MODULE))

typedef struct _BTestModule      BTestModule;
typedef struct _BTestModuleClass BTestModuleClass;

struct _BTestModule
{
  BModule   parent_instance;

  gint      x, y;
  gboolean  on;
};

struct _BTestModuleClass
{
  BModuleClass  parent_class;
};

static GType     b_test_module_get_type   (GTypeModule       *module);
static void      b_test_module_class_init (BTestModuleClass  *klass);
static void      b_test_module_init       (BTestModule       *test_module);
static void      b_test_module_finalize   (GObject           *object);
static gboolean  b_test_module_query      (gint               width,
                                           gint               height,
                                           gint               channels,
                                           gint               maxval);
static gboolean  b_test_module_prepare    (BModule           *module,
                                           GError           **error);
static void      b_test_module_relax      (BModule           *module);
static void      b_test_module_start      (BModule           *module);
static void      b_test_module_stop       (BModule           *module);
static void      b_test_module_event      (BModule           *module,
                                           BModuleEvent      *event);
static gint      b_test_module_tick       (BModule           *module);
static void      b_test_module_describe   (BModule           *module,
                                           const gchar      **title,
                                           const gchar      **description,
                                           const gchar      **author);



static BModuleClass * parent_class       = NULL;
static GType          b_type_test_module = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_test_module_get_type (module);

  return TRUE;
}

GType
b_test_module_get_type (GTypeModule *module)
{
  if (! b_type_test_module)
    {
      static const GTypeInfo test_module_info =
      {
        sizeof (BTestModuleClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_test_module_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BTestModule),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_test_module_init,
      };

      /* !!!!!!!!! The name given in the next function MUST be unique! */

      b_type_test_module = g_type_module_register_type (module,
                                                        B_TYPE_MODULE,
                                                        "BTestModule",
                                                        &test_module_info,
                                                        0);
    }

  return b_type_test_module;
}

static void
b_test_module_class_init (BTestModuleClass *klass)
{
  GObjectClass *object_class;
  BModuleClass *module_class;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize = b_test_module_finalize;

  module_class->query    = b_test_module_query;
  module_class->prepare  = b_test_module_prepare;
  module_class->relax    = b_test_module_relax;
  module_class->start    = b_test_module_start;
  module_class->stop     = b_test_module_stop;
  module_class->event    = b_test_module_event;
  module_class->tick     = b_test_module_tick;
  module_class->describe = b_test_module_describe;
}

static void
b_test_module_init (BTestModule *test_module)
{
  /* the device-independant initialization can happen here */
  test_module->on = FALSE;
}

static void
b_test_module_finalize (GObject *object)
{
  BTestModule *test_module = B_TEST_MODULE (object);

  /* if we had allocated any resources, we'd free them now */

  /* just do something to please the compiler ... */
  test_module->on = FALSE;

  /* you MUST upchain here! */
  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static gboolean
b_test_module_query (gint     width,
                     gint     height,
                     gint     channels,
                     gint     maxval)
{
  return (width > 0 && height > 0 && channels == 1 && maxval > 0);
}

static gboolean
b_test_module_prepare (BModule  *module,
                       GError  **error)
{
  BTestModule *test_module = B_TEST_MODULE (module);

  /* initialize the module values that depend on the output device */
  test_module->x = module->width  / 2;
  test_module->y = module->height / 2;

  return TRUE;
}

static void
b_test_module_relax (BModule *module)
{
  g_print ("b_test_module_relax\n");

  /* if we had allocated any resources in prepare, we'd free them now */
}

static void
b_test_module_start (BModule *module)
{
  g_print ("b_test_module_start\n");

  /* clear the screen */
  b_module_fill (module, 0);
  b_module_paint (module);

  /* start the tick machinery */
  b_module_ticker_start (module, 200); 
}

static void
b_test_module_stop (BModule *module)
{
  g_print ("b_test_module_stop\n");

  /* clear the screen */
  b_module_fill (module, 0);
  b_module_paint (module);
}

static void
b_test_module_event (BModule      *module,
                     BModuleEvent *event)
{
  BTestModule *test_module;

  g_print ("b_test_module_event\n");

  test_module = B_TEST_MODULE (module);

  switch (event->device_id)
    {
    case 0:
      switch (event->type)
        {
        case B_EVENT_TYPE_KEY:
          switch (event->key)
            {
            case B_KEY_1:
              if (test_module->x > 0)
                test_module->x--;	 
            case B_KEY_2:
              if (test_module->y > 0)
                test_module->y--;	
              break;

            case B_KEY_7:
              if (test_module->y < module->height-1)
                test_module->y++;
            case B_KEY_4:
              if (test_module->x > 0)
                test_module->x--;	 
              break;

            case B_KEY_3:
              if (test_module->y > 0)
                test_module->y--;	
            case B_KEY_6:
              if (test_module->x < module->width-1)
                test_module->x++;
              break;

            case B_KEY_9:
              if (test_module->x < module->width-1)
                test_module->x++;
            case B_KEY_8:
              if (test_module->y < module->height-1)
                test_module->y++;
              break;

            case B_KEY_0:
              test_module->x = module->width  / 2;
              test_module->y = module->height / 2;
              break;

            default:
              break;
            }
          break;
        default:
          /* other kinds of events should not occur */
          break;
        }
    case 1:
      /* if this was a multiplayer game, you could dispatch events
         from the other 'panel' here */
      break;
    }

  /* fill the buffer with zeros */
  b_module_fill (module, 0);

  /* draw the point */
  if (test_module->on)
    b_module_draw_point (module,
                         test_module->x, test_module->y, module->maxval);

  b_module_paint (module);
}

static gint
b_test_module_tick (BModule *module)
{
  BTestModule *test_module;

  g_print ("b_test_module_tick\n");

  test_module = B_TEST_MODULE (module);

  /* invert the state */
  test_module->on = (test_module->on == FALSE);

  /* draw the point */
  b_module_draw_point (module,
                       test_module->x, test_module->y,
                       test_module->on ? module->maxval : 0);

  b_module_paint (module);

  /* we want to be called again in 200 milliseconds */
  return 200;
}

static void
b_test_module_describe (BModule      *module,
                        const gchar **title,
                        const gchar **description,
                        const gchar **author)
{
  *title       = "Test";
  *description = "Blib test module and code example";
  *author      = "Blinkenlights Crew";
}

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

#define TIMEOUT 75


typedef enum
{
  DIR_FTTB,
  DIR_FRTL,
  DIR_FBTT,
  DIR_FLTR,
  DIR_LAST
} BPushlineDir;

enum
{
  MODE_EAT
};

enum
{
  PROP_0,
  PROP_DIR,
};


#define DEFAULT_MODE MODE_EAT

#define B_TYPE_PUSHLINE_DIR     (b_pushline_dir_get_type ())

#define B_TYPE_PUSHLINE         (b_type_pushline)
#define B_PUSHLINE(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_PUSHLINE, BPushline))
#define B_PUSHLINE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_PUSHLINE, BPushlineClass))
#define B_IS_PUSHLINE(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_PUSHLINE))

typedef struct _BPushline      BPushline;
typedef struct _BPushlineClass BPushlineClass;

struct _BPushline
{
  BModule   parent_instance;

  gint      dir;
  gint      mode;
  gint      pos;
};

struct _BPushlineClass
{
  BModuleClass  parent_class;
};

static GType    b_pushline_get_type     (GTypeModule     *module);
static void     b_pushline_class_init   (BPushlineClass  *klass);
static void     b_pushline_init         (BPushline       *pushline);
static void     b_pushline_set_property (GObject         *object,
                                         guint            property_id,
                                         const GValue    *value,
                                         GParamSpec      *pspec);
static gboolean b_pushline_query        (gint             width,
                                         gint             height,
                                         gint             channels,
                                         gint             maxval);
static gboolean b_pushline_prepare      (BModule         *module,
                                         GError         **error);
static void     b_pushline_relax        (BModule         *module);
static void     b_pushline_start        (BModule         *module);
static gint     b_pushline_tick         (BModule         *module);
static void     b_pushline_describe     (BModule         *module,
                                         const gchar    **title,
                                         const gchar    **description,
                                         const gchar    **author);


static GType  b_type_pushline = 0;


static const GEnumValue pushline_dir_enum_values[] =
{
  { DIR_FTTB, "top-to-bottom", NULL },
  { DIR_FRTL, "right-to-left", NULL },
  { DIR_FBTT, "bottom-to-top", NULL },
  { DIR_FLTR, "left-to-right", NULL },
  { 0, NULL, NULL }
};

GType
b_pushline_dir_get_type (void)
{
  static GType enum_type = 0;

  if (!enum_type)
    enum_type = g_enum_register_static ("BPushlineDirection",
                                        pushline_dir_enum_values);

  return enum_type;
}

G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_pushline_get_type (module);
  return TRUE;
}

GType
b_pushline_get_type (GTypeModule *module)
{
  if (!b_type_pushline)
    {
      static const GTypeInfo pushline_info =
	{
	  sizeof (BPushlineClass),
	  NULL,           /* base_init */
	  NULL,           /* base_finalize */
	  (GClassInitFunc) b_pushline_class_init,
	  NULL,           /* class_finalize */
	  NULL,           /* class_data */
	  sizeof (BPushline),
	  0,              /* n_preallocs */
	  (GInstanceInitFunc) b_pushline_init,
	};
     
      b_type_pushline = g_type_module_register_type (module, B_TYPE_MODULE,
                                                     "BPushline",
						     &pushline_info, 0);
    }
 
  return b_type_pushline;
}

static void
b_pushline_class_init (BPushlineClass *klass)
{
  GObjectClass *object_class;
  BModuleClass *module_class;
  GParamSpec   *param_spec;
  gint          dir;

  dir = random () % DIR_LAST;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  object_class->set_property = b_pushline_set_property;

  param_spec = g_param_spec_enum ("direction", NULL,
                                 "The direction the line moves.",
                                  B_TYPE_PUSHLINE_DIR, dir,
                                  G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_DIR, param_spec);

  module_class->query    = b_pushline_query;
  module_class->prepare  = b_pushline_prepare;
  module_class->relax    = b_pushline_relax;
  module_class->start    = b_pushline_start;
  module_class->tick     = b_pushline_tick;
  module_class->describe = b_pushline_describe;
}

static void
b_pushline_init (BPushline *pushline)
{
  pushline->mode = DEFAULT_MODE;
}

static void
b_pushline_set_property (GObject      *object,
			 guint         property_id,
			 const GValue *value,
			 GParamSpec   *pspec)
{
  BPushline *line = B_PUSHLINE (object);

  switch (property_id)
    {
    case PROP_DIR:
      line->dir = g_value_get_enum (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
b_pushline_query (gint width,
		  gint height,
		  gint channels,
		  gint maxval)
{
  return (width > 0 && height > 3 && channels == 1 && maxval == 255);
}

static gboolean
b_pushline_prepare (BModule  *module,
		    GError  **error)
{
  BPushline *pushline = B_PUSHLINE (module);

  switch (pushline->dir)
    {
    case DIR_FTTB:
      pushline->pos = -1;
      break;
    case DIR_FRTL:
      pushline->pos = module->width;
      break;
    case DIR_FBTT:
      pushline->pos = module->height;
      break;
    case DIR_FLTR:
      pushline->pos = -1;
      break;
    }
 
  return TRUE;
}

static void
b_pushline_relax (BModule *module)
{
}

static void
b_pushline_start (BModule *module)
{
  b_module_ticker_start (module, b_pushline_tick (module));
}

static gint
b_pushline_tick (BModule *module)
{
  BPushline *pushline = B_PUSHLINE (module);
  gint       width    = module->width;
  gint       height   = module->height;

  switch (pushline->dir)
    {
    case DIR_FTTB:
      pushline->pos++;
      if (pushline->pos >= width)
	goto stop;

      b_module_draw_line (module,
                          0, pushline->pos, width - 1, pushline->pos,
                          module->maxval);
      if (pushline->pos > 0)
	b_module_draw_line (module,
                            0, pushline->pos - 1, width - 1, pushline->pos - 1,
                            0);
      break;

    case DIR_FRTL:
      pushline->pos--;
      if (pushline->pos < 0)
	goto stop;

      b_module_draw_line (module,
                          pushline->pos, 0, pushline->pos, height - 1,
                          module->maxval);
      if (pushline->pos < width - 1)
	b_module_draw_line (module,
                            pushline->pos + 1, 0, pushline->pos + 1, height -1,
                            0);
      break;

    case DIR_FBTT:
      pushline->pos--;
      if (pushline->pos < 0)
	goto stop;

      b_module_draw_line (module,
                          0, pushline->pos, width-1, pushline->pos,
                          module->maxval);
      if (pushline->pos < height-1)
	b_module_draw_line (module,
                            0, pushline->pos + 1, width - 1, pushline->pos + 1,
                            0);
      break;

    case DIR_FLTR:
      pushline->pos++;
      if (pushline->pos > width)
	goto stop;
      b_module_draw_line (module,
                          pushline->pos, 0, pushline->pos, height - 1,
                          module->maxval);
      if (pushline->pos > 0)
	b_module_draw_line (module,
                            pushline->pos - 1, 0, pushline->pos - 1, height -1,
                            0);
      break;
    }

  b_module_paint (module);
  return TIMEOUT;

 stop:
  b_module_fill (module, 0);
  b_module_paint (module);
  b_module_request_stop (module);

  return 0;
}

static void
b_pushline_describe (BModule      *module,
		     const gchar **title,
		     const gchar **description,
		     const gchar **author)
{
  *title       = "BPushline";
  *description = "Pushline hack";
  *author      = "Daniel Mack";
}

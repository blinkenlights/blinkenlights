/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2002  The Blinkenlights Crew
 *                     Sven Neumann <sven@gimp.org>
 *
 * Inspired by the Qix hack from the xscreensaver package.
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


#define TIMEOUT  60

#define SPREAD   100  /* prescaled! */
#define SCALE    6


#define B_TYPE_QIX         (b_type_qix)
#define B_QIX(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_QIX, BQix))
#define B_QIX_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_QIX, BQixClass))
#define B_IS_QIX(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_QIX))

typedef struct _BQix      BQix;
typedef struct _BQixClass BQixClass;

typedef struct _BQixLine  BQixLine;

struct _BQixLine
{
  struct {
    gint     x;
    gint     y;
    gint     dx;
    gint     dy;
  } points[2];
};

struct _BQix
{
  BModule   parent_instance;

  gint      max_lines;

  BQixLine *lines;
  guchar   *values;
  gint      nlines;
  gint      maxx;
  gint      maxy;
};

struct _BQixClass
{
  BModuleClass  parent_class;
};

enum
{
  PROP_0,
  PROP_LINES
};


static GType    b_qix_get_type     (GTypeModule   *module);
static void     b_qix_class_init   (BQixClass     *klass);
static void     b_qix_finalize     (GObject       *object);
static void     b_qix_set_property (GObject       *object,
                                    guint          property_id,
                                    const GValue  *value,
                                    GParamSpec    *pspec);
static void     b_qix_init         (BQix          *qix);
static gboolean b_qix_query        (gint           width,
                                    gint           height,
                                    gint           channels,
                                    gint           maxval);
static gboolean b_qix_prepare      (BModule       *module,
                                    GError       **error);
static void     b_qix_relax        (BModule       *module);
static void     b_qix_start        (BModule       *module);
static gint     b_qix_tick         (BModule       *module);
static void     b_qix_describe     (BModule       *module,
                                    const gchar  **title,
                                    const gchar  **description,
                                    const gchar  **author);


static BModuleClass * parent_class = NULL;
static GType          b_type_qix   = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_qix_get_type (module);
  return TRUE;
}

GType
b_qix_get_type (GTypeModule *module)
{
  if (!b_type_qix)
    {
      static const GTypeInfo qix_info =
      {
        sizeof (BQixClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_qix_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BQix),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_qix_init,
      };

      b_type_qix = g_type_module_register_type (module,
                                                B_TYPE_MODULE, "BQix",
                                                &qix_info, 0);
    }

  return b_type_qix;
}

static void
b_qix_class_init (BQixClass *klass)
{
  GObjectClass *object_class;
  BModuleClass *module_class;
  GParamSpec   *param_spec;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize     = b_qix_finalize;
  object_class->set_property = b_qix_set_property;

  param_spec = g_param_spec_int ("lines", NULL,
                                 "The number of lines to draw.",
                                 1, 64, 5,
                                 G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_LINES, param_spec);

  module_class->query    = b_qix_query;
  module_class->prepare  = b_qix_prepare;
  module_class->relax    = b_qix_relax;
  module_class->start    = b_qix_start;
  module_class->tick     = b_qix_tick;
  module_class->describe = b_qix_describe;
}

static void
b_qix_finalize (GObject *object)
{
  BQix *qix = B_QIX (object);

  g_free (qix->lines);
  g_free (qix->values);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
b_qix_set_property (GObject      *object,
                    guint         property_id,
                    const GValue *value,
                    GParamSpec   *pspec)
{
  BQix *qix = B_QIX (object);

  switch (property_id)
    {
    case PROP_LINES:
      g_return_if_fail (g_value_get_int (value) > 0);
      qix->max_lines = g_value_get_int (value);
      break;

    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
b_qix_init (BQix *qix)
{
}

static gboolean
b_qix_query (gint     width,
             gint     height,
             gint     channels,
             gint     maxval)
{
  return (width > 0 && height > 0 && channels == 1 && maxval > 0);
}

static gboolean
b_qix_prepare (BModule  *module,
               GError  **error)
{
  BQix *qix = B_QIX (module);
  gint  i, value;
  gint  xspread, yspread;

  if (qix->max_lines < 1)
    {
      g_set_error (error, 0, 0, "Qix module must have at least one line");
      return FALSE;
    }

  qix->lines  = g_new (BQixLine, 2 * qix->max_lines);
  qix->values = g_new (guchar, qix->max_lines);

  if (module->aspect < 1.0)
    {
      xspread = SPREAD;
      yspread = module->aspect * xspread;
    }
  else
    {
      yspread = SPREAD;
      xspread = module->aspect * yspread;
    }

  qix->maxx = (module->width  << SCALE) - 1;
  qix->maxy = (module->height << SCALE) - 1;

  qix->nlines = 1;

  for (i = 0; i < 2; i++)
    {
      qix->lines[0].points[i].x = rand () % qix->maxx;
      qix->lines[0].points[i].y = rand () % qix->maxy;
      qix->lines[0].points[i].dx = (rand () % (xspread + 1)) - (xspread / 2);
      qix->lines[0].points[i].dy = (rand () % (yspread + 1)) - (yspread / 2);
    }

  value = module->maxval / qix->max_lines;

  for (i = 0; i < qix->max_lines; i++)
    qix->values[i] = module->maxval - i * value;

  return TRUE;
}

static void
b_qix_relax (BModule *module)
{
  BQix *qix = B_QIX (module);

  if (qix->lines)
    {
      g_free (qix->lines);
      qix->lines = NULL;
    }
  if (qix->values)
    {
      g_free (qix->values);
      qix->values = NULL;
    }
}

static void
b_qix_start (BModule *module)
{
  b_module_ticker_start (module, TIMEOUT);
}

static gint
b_qix_tick (BModule *module)
{
  BQix     *qix = B_QIX (module);
  BQixLine *line;
  gint      i;

  b_module_fill (module, 0);

#define wiggle(point, delta, max) \
  point += delta; \
  if (point < 0) point = 0, delta = -delta, point += delta / 2; \
  else if (point > max) point = max, delta = -delta, point += delta / 2;

  for (i = qix->nlines - 1; i > 0; i--)
    memcpy (qix->lines + i, qix->lines + i - 1, sizeof (BQixLine));

  line = qix->lines;

  for (i = 0; i < 2; i++)
    {
      wiggle (line->points[i].x, line->points[i].dx, qix->maxx);
      wiggle (line->points[i].y, line->points[i].dy, qix->maxy);
    }

  i = qix->nlines - 1;
  if (i % 2)
    i--;

  for (; i >= 0; i -= 2)
    {
      line = qix->lines + i;
      b_module_draw_line (module,
                          line->points[0].x >> SCALE,
                          line->points[0].y >> SCALE,
                          line->points[1].x >> SCALE,
                          line->points[1].y >> SCALE,
                          qix->values[i / 2]);
    }

  b_module_paint (module);

  if (qix->nlines < 2 * qix->max_lines)
    qix->nlines++;

  return TIMEOUT;
}

static void
b_qix_describe (BModule      *module,
                const gchar **title,
                const gchar **description,
                const gchar **author)
{
  *title       = "BQix";
  *description = "Qix hack";
  *author      = "Sven Neumann";
}

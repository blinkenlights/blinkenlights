/* blccc - Blinkenlights Chaos Control Center
 *
 * Copyright (c) 2001-2002  Sven Neumann <sven@gimp.org>
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
#include <unistd.h>

#include <blib/blib.h>

#include "bltypes.h"
#include "blloveletters.h"
#include "blondemand.h"


#define TICK 250

enum
{
  PROP_0,
  PROP_ADVANCED,
  PROP_LOVELETTERS
};

static void      bl_on_demand_class_init   (BlOnDemandClass  *class);
static void      bl_on_demand_finalize     (GObject          *object);
static void      bl_on_demand_set_property (GObject          *object,
                                            guint             property_id,
                                            const GValue     *value,
                                            GParamSpec       *pspec);
static gboolean  bl_on_demand_query        (gint              width,
                                            gint              height,
                                            gint              channels,
                                            gint              maxval);
static gboolean  bl_on_demand_prepare      (BModule          *module,
                                            GError          **error);
static void      bl_on_demand_relax        (BModule          *module);
static void      bl_on_demand_start        (BModule          *module);
static gint      bl_on_demand_tick         (BModule          *module);
static void      bl_on_demand_event        (BModule          *module,
                                            BModuleEvent     *event);
static void      bl_on_demand_describe     (BModule          *module,
                                            const gchar     **title,
                                            const gchar     **description,
                                            const gchar     **author);
static void      bl_on_demand_paint        (BModule          *module);


static BModuleClass *parent_class = NULL;


GType
bl_on_demand_get_type (void)
{
  static GType type = 0;

  if (! type)
    {
      static const GTypeInfo info =
      {
        sizeof (BlOnDemandClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) bl_on_demand_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BlOnDemand),
        0,              /* n_preallocs */
        NULL            /* instance_init */
      };

      type = g_type_register_static (B_TYPE_MOVIE_PLAYER,
                                     "BlOnDemand", &info, 0);
    }

  return type;
}

static void
bl_on_demand_class_init (BlOnDemandClass *klass)
{
  GObjectClass      *object_class;
  GParamSpec        *param_spec;
  BModuleClass      *module_class;
  BMoviePlayerClass *player_class;

  object_class = G_OBJECT_CLASS (klass);
  module_class = B_MODULE_CLASS (klass);
  player_class = B_MOVIE_PLAYER_CLASS (klass);

  parent_class = g_type_class_peek_parent (klass);

  object_class->finalize     = bl_on_demand_finalize;
  object_class->set_property = bl_on_demand_set_property;

  param_spec = g_param_spec_boolean ("advanced", NULL,
                                     "Use advanced features (pause/restart).",
                                     FALSE,
                                     G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_ADVANCED, param_spec);

  param_spec = b_param_spec_filename ("loveletters", NULL,
                                      "The filename of the loveletters list.",
                                      NULL,
                                      G_PARAM_CONSTRUCT | G_PARAM_WRITABLE);
  g_object_class_install_property (object_class, PROP_LOVELETTERS, param_spec);

  module_class->max_players = 1;

  module_class->query    = bl_on_demand_query;
  module_class->prepare  = bl_on_demand_prepare;
  module_class->relax    = bl_on_demand_relax;
  module_class->start    = bl_on_demand_start;
  module_class->tick     = bl_on_demand_tick;
  module_class->event    = bl_on_demand_event;
  module_class->describe = bl_on_demand_describe;

  player_class->request_stop = parent_class->start; /* loop the movie */
}

static void
bl_on_demand_finalize (GObject *object)
{
  BlOnDemand *on_demand = BL_ON_DEMAND (object);

  if (on_demand->filename)
    {
      g_free (on_demand->filename);
      on_demand->filename = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
bl_on_demand_set_property (GObject      *object,
                           guint         property_id,
                           const GValue *value,
                           GParamSpec   *pspec)
{
  BlOnDemand *on_demand = BL_ON_DEMAND (object);

  switch (property_id)
    {
    case PROP_ADVANCED:
      on_demand->advanced = g_value_get_boolean (value);
      break;
    case PROP_LOVELETTERS:
      if (on_demand->filename)
        g_free (on_demand->filename);
      on_demand->filename = g_value_dup_string (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static gboolean
bl_on_demand_query (gint  width,
                    gint  height,
                    gint  channels,
                    gint  maxval)
{
  return (width > 2 * MAX_ON_DEMAND_CHARS && height > 3 &&
          channels == 1 && maxval == 255);
}

static gboolean
bl_on_demand_prepare (BModule  *module,
                      GError  **error)
{
  BlOnDemand *on_demand = BL_ON_DEMAND (module);

  if (!on_demand->filename)
    {
      g_set_error (error, 0, 0, "No loveletters file configured");
      return FALSE;
    }

  B_MOVIE_PLAYER (module)->clear = FALSE;

  on_demand->loveletters =
    bl_loveletters_new_from_file (on_demand->filename, error);

  on_demand->movie_active = FALSE;
  on_demand->movie_paused = FALSE;

  return (on_demand->loveletters != NULL);
}

static void
bl_on_demand_relax (BModule *module)
{
  BlOnDemand *on_demand = BL_ON_DEMAND (module);

  if (on_demand->loveletters)
    {
      g_object_unref (on_demand->loveletters);
      on_demand->loveletters = NULL;
    }

  if (on_demand->movie_active)
    {
      parent_class->relax (module);
      on_demand->movie_active = FALSE;
    }
}

static void
bl_on_demand_start (BModule *module)
{
  BlOnDemand *on_demand = BL_ON_DEMAND (module);

  on_demand->chars        = -1;
  on_demand->movie_active = FALSE;

  bl_on_demand_paint (module);
  b_module_ticker_start (module, TICK);
}

static gint
bl_on_demand_tick (BModule *module)
{
  BlOnDemand *on_demand = BL_ON_DEMAND (module);
  
  if (on_demand->movie_active && !on_demand->movie_paused)
    return parent_class->tick (module);

  if (!on_demand->movie_active) 
    bl_on_demand_paint (module);

  return TICK;
}

static void
bl_on_demand_event (BModule      *module,
                    BModuleEvent *event)
{
  BlOnDemand  *on_demand = BL_ON_DEMAND (module);
  const gchar *movie;

  /* in DAU mode, ignore keys when a movie has been started */
  if (!on_demand->advanced && on_demand->movie_active)
    return;

  if (event->type != B_EVENT_TYPE_KEY)
    return;

  switch (event->key)
    {
    case B_KEY_0 ... B_KEY_9:
      if (on_demand->chars < 0)
        return;

      on_demand->code[on_demand->chars] = event->key + '0';
      on_demand->chars++;

      if (on_demand->chars < MAX_ON_DEMAND_CHARS)
        break;
      /* else fallthru */

    case B_KEY_HASH:
      if (on_demand->movie_active)
        {
          on_demand->movie_paused = (on_demand->movie_paused == FALSE);

          if (!on_demand->movie_paused)
            {
              b_module_fill (module, 0);
              parent_class->start (module);
            }
        }

      if (on_demand->chars <= 0)
        return;

      if (on_demand->movie_active)
        {
          parent_class->relax (module);
          on_demand->movie_active = FALSE;
        }

      on_demand->code[on_demand->chars] = '\0';
      on_demand->chars = -1;

      movie = bl_loveletters_lookup (on_demand->loveletters, on_demand->code);
      if (movie)
        {
          GError *error = NULL;

          g_object_set (module, "movie", movie, NULL);

          if (parent_class->prepare (module, &error))
            {
              g_printerr ("Starting loveletter for ID '%s'.\n",
                          on_demand->code);

              on_demand->movie_active = TRUE;
              on_demand->movie_paused = FALSE;

              b_module_fill (module, 0);
              parent_class->start (module);
              return;
            }
          else
            {
              g_printerr ("Error preparing BlOnDemand: %s\n", error->message);
              g_clear_error (&error);

              on_demand->movie_active = FALSE;
            }
        }
      else
        {
          g_printerr ("No loveletter found for ID '%s'.\n", on_demand->code); 
        }
      break;

    case B_KEY_ASTERISK:
      on_demand->chars = 0;
      if (on_demand->movie_active)
        {
          B_MOVIE_PLAYER (module)->current = NULL;
          on_demand->movie_paused = TRUE;
        }
      break;
    }

  bl_on_demand_paint (module);
}

static void
bl_on_demand_describe (BModule      *module,
                       const gchar **title,
                       const gchar **description,
                       const gchar **author)
{
  *title  = "BlOnDemand";
  *author = "Sven Neumann";

  if (BL_ON_DEMAND (module)->advanced)
    *description = "Interactive movie player (advcanced)";
  else
    *description = "Interactive movie player (DAU mode)";
}

static void
bl_on_demand_paint (BModule *module)
{
  gint n;
  gint w = module->width  / 2;
  gint h = module->height / 2;
  gint v = module->maxval / 2;
  gint l = MIN (w / 2, h / 2);

  b_module_fill (module, 0);

  h--;

  n = BL_ON_DEMAND (module)->chars;

  if (n < 0)
    {
      b_module_draw_line (module, w - l, h    , w + l, h    , v + rand()%v);
      b_module_draw_line (module, w - l, h - l, w + l, h + l, v + rand()%v);
      b_module_draw_line (module, w - l, h + l, w + l, h -l , v + rand()%v);
    }
  else
    {
      guchar *d = module->buffer + (h / 2) * module->width;
      gint x, i;

      x = w - MAX_ON_DEMAND_CHARS;

      for (i = 0; i < MAX_ON_DEMAND_CHARS; i++, x += 2)
        d[x] = (i < n) ? module->maxval : module->maxval / 2;
    }

  if (n > 0 || BL_ON_DEMAND (module)->movie_active)
    {
      b_module_draw_line (module,
                          w - l  , h + l/2, w + l  , h + l/2  , v + rand()%v);
      b_module_draw_line (module, 
                          w - l  , h + l  , w + l  , h + l    , v + rand()%v);
      b_module_draw_line (module,
                          w - l/2, h      , w - l/2, h + 3*l/2, v + rand()%v);
      b_module_draw_line (module,
                          w + l/2, h      , w + l/2, h + 3*l/2, v + rand()%v);
    }

  b_module_paint (module);
}

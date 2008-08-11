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

#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include <glib.h>
#include <gmodule.h>

#include <blib/blib.h>


#define START_SPEED    600
#define ACC_ON_STUCK   4
#define FLASH_SPEED    55
#define FALL_SPEED     50
#define BOTTOM_OFFSET  0   /* we used a value of 2 for Arcade/Paris */

/*  #define TETRIS_DEBUG 1  */


#define B_TYPE_TETRIS         (b_type_tetris)
#define B_TETRIS(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_TETRIS, BTetris))
#define B_TETRIS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_TETRIS, BTetrisClass))
#define B_IS_TETRIS(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_TETRIS))

typedef struct _BTetris      BTetris;
typedef struct _BTetrisClass BTetrisClass;

struct _BTetris
{
  BModule  parent_instance;

  gint      x, y;
  gint      rotation;
  guchar   *field;
  gint      field_size;
  gint      speed;
  gint      tile;
  gboolean  game_over;

  gboolean *completed;
  gint      n_completed;

  gboolean  falling;

  gboolean  flash_state;

  gint      player_device_id;
};

struct _BTetrisClass
{
  BModuleClass  parent_class;
};

static GType      b_tetris_get_type   (GTypeModule   *module);


static void       b_tetris_class_init (BTetrisClass  *klass);
static void       b_tetris_init       (BTetris       *tetris);

static gboolean   b_tetris_query      (gint           width,
                                       gint           height,
                                       gint           channels,
                                       gint           maxval);
static gboolean   b_tetris_prepare    (BModule       *module,
                                       GError       **error);
static void       b_tetris_relax      (BModule       *module);
static void       b_tetris_start      (BModule       *module);
static void       b_tetris_stop       (BModule       *module);
static void       b_tetris_event      (BModule       *module,
                                       BModuleEvent  *event);
static gint       b_tetris_tick       (BModule       *module);
static void       b_tetris_describe   (BModule       *module,
                                       const gchar  **title,
                                       const gchar  **description,
                                       const gchar  **author);


static GType b_type_tetris = 0;

typedef struct _TetrisTile
{
  gchar  data[4][16];
  guchar color;
} TetrisTile;

static TetrisTile tile[] =
{
  {
    {
      { 0, 0, 0, 0,
	1, 1, 1, 1,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 0, 1, 0, 0,
	0, 1, 0, 0,
	0, 1, 0, 0,
	0, 1, 0, 0 },
      { 0, 0, 0, 0,
	1, 1, 1, 1,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 0, 1, 0, 0,
	0, 1, 0, 0,
	0, 1, 0, 0,
	0, 1, 0, 0 },
    },
    0x77
  },
  {
    {
      { 1, 1, 0, 0,
	1, 1, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 1, 1, 0, 0,
	1, 1, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 1, 1, 0, 0,
	1, 1, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 1, 1, 0, 0,
	1, 1, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
    },
    0x99
  },
  {
    {
      { 1, 0, 0, 0,
	1, 1, 1, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 1, 1, 0, 0,
	1, 0, 0, 0,
	1, 0, 0, 0,
	0, 0, 0, 0 },
      { 1, 1, 1, 0,
	0, 0, 1, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 0, 1, 0, 0,
	0, 1, 0, 0,
	1, 1, 0, 0,
	0, 0, 0, 0 },
    },
    0xbb
  },
  {
    {
      { 0, 0, 1, 0,
	1, 1, 1, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 1, 0, 0, 0,
	1, 0, 0, 0,
	1, 1, 0, 0,
	0, 0, 0, 0 },
      { 1, 1, 1, 0,
	1, 0, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 1, 1, 0, 0,
	0, 1, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 0 },
    },
    0xbb
  },
  {
    {
      { 1, 1, 1, 0,
	0, 1, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 0, 1, 0, 0,
	1, 1, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 0 },
      { 0, 1, 0, 0,
	1, 1, 1, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 1, 0, 0, 0,
	1, 1, 0, 0,
	1, 0, 0, 0,
	0, 0, 0, 0 },
    },
    0xdd
  },
  {
    {
      { 0, 1, 1, 0,
	1, 1, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 1, 0, 0, 0,
	1, 1, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 0 },
      { 0, 1, 1, 0,
	1, 1, 0, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 1, 0, 0, 0,
	1, 1, 0, 0,
	0, 1, 0, 0,
	0, 0, 0, 0 },
    },
    0xff
  },
  {
    {
      { 1, 1, 0, 0,
	0, 1, 1, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 0, 1, 0, 0,
	1, 1, 0, 0,
	1, 0, 0, 0,
	0, 0, 0, 0 },
      { 1, 1, 0, 0,
	0, 1, 1, 0,
	0, 0, 0, 0,
	0, 0, 0, 0 },
      { 0, 1, 0, 0,
	1, 1, 0, 0,
	1, 0, 0, 0,
	0, 0, 0, 0 },
    },
    0xff
  }
};

static void
b_tetris_blend_down (BModule *module)
{
  gint x, y, v;
  guchar *pixel;
 
  for (y = 0; y < module->height; y++)
    for (v = 255; v >= 0; v -= 0xf)
      {
	for (x = 0; x < module->width; x++)
	  {
	    pixel = module->buffer + (y * module->width + x);
	    if (*pixel > v)
	      *pixel = v;
	  }

	b_module_paint (module);
        usleep (5000);
      }
}

static gint
b_tetris_new_tile (void)
{
  return random () % (sizeof (tile) / sizeof (tile[0]));
}

static gint
b_tetris_tile_width (gint tile_n,
                     gint rotation)
{
  gint x, y, w = 0;

  for (x = 0; x < 4; x++)
    for (y = 0; y < 4; y++)
      if (tile[tile_n].data[rotation][y*4+x])
	w = x + 1;

  return w;
}

static gint
b_tetris_tile_height (gint tile_n,
                      gint rotation)
{
  gint x, y, h = 0;

  for (y = 0; y < 4; y++)
    for (x = 0; x < 4; x++)
      if (tile[tile_n].data[rotation][y*4+x])
	h = y + 1;

  return h;
}

static gint
b_tetris_tile_left_offset (gint tile_n,
                           gint rotation)
{
  gint o = 4, x, y;

  for (y = 0; y < 4; y++)
    for (x = 3; x >= 0; x--)
      if (tile[tile_n].data[rotation][y*4+x])
	if (o > x)
	  o = x;

  return o;
}

static void
b_tetris_paint (BModule *module)
{
  BTetris *tetris;
  gchar   *tile_data;
  gint     x, y;
  gint     tile_width, tile_height;

  tetris = B_TETRIS (module);

  memcpy (module->buffer, tetris->field, tetris->field_size);

  tile_data   = tile[tetris->tile].data[tetris->rotation];
  tile_width  = b_tetris_tile_width  (tetris->tile, tetris->rotation);
  tile_height = b_tetris_tile_height (tetris->tile, tetris->rotation);

#ifdef TETRIS_DEBUG
  g_print ("b_tetris_paint: %d x %d\n", tile_width, tile_height);
#endif

  if (tetris->n_completed == 0)
    {
      for (x = 0; x < tile_width; x++)
	for (y = 0; y < tile_height; y++)
	  {
	    if (tile_data[y * 4 + x])
	      {
		guchar *data = (module->buffer + 
				module->width * (tetris->y + y) +
				(tetris->x + x) * 2);

		data[0] = tile[tetris->tile].color;
		data[1] = tile[tetris->tile].color;
	      }
	  }
    }
 
  b_module_paint (module);
}

static void
b_tetris_rotate (BModule  *module,
                 gboolean  clockwise)
{
  BTetris     *tetris = B_TETRIS (module);
  const gchar *tile_data;
  gint         x, y;
  gint         tile_width, tile_height, tile_leftoff;
  gint         rotation;

  rotation = tetris->rotation;

  if (clockwise)
    rotation += 2;

  rotation++;
  rotation %= 4;

  tile_data    = tile[tetris->tile].data[rotation];
  tile_width   = b_tetris_tile_width  (tetris->tile, rotation);
  tile_height  = b_tetris_tile_height (tetris->tile, rotation);
  tile_leftoff = b_tetris_tile_left_offset (tetris->tile, rotation);

  if (tetris->y + tile_height > module->height - BOTTOM_OFFSET)
    return;

  /* check if rotated tile collides with s.th. */
  for (y = 0; y < tile_height; y++)
    {
      for (x = 0; x < tile_width; x++)
	if (tile_data[y * 4 + x] &&
	    tetris->field[module->width *
			  (tetris->y + tile_height) + (tetris->x + x) * 2])
	  return;
    }

  while (tetris->x + tile_width > module->width/2)
    tetris->x--;

  while (tetris->x - tile_leftoff +  1 < 0)
    tetris->x++;

  tetris->rotation = rotation;
  b_tetris_paint (module);
}

static gboolean
b_tetris_down (BModule *module)
{
  BTetris     *tetris = B_TETRIS (module);
  const gchar *tile_data;
  gboolean     stuck = FALSE;
  gint         x, y;
  gint         tile_width, tile_height;

  tile_data   = tile[tetris->tile].data[tetris->rotation];
  tile_width  = b_tetris_tile_width  (tetris->tile, tetris->rotation);
  tile_height = b_tetris_tile_height (tetris->tile, tetris->rotation);

  if (tetris->y >= module->height - tile_height - BOTTOM_OFFSET)
    {
      stuck = TRUE;
    }
  else
    {
      for (x = 0; x < tile_width; x++)
	for (y = 0; y < tile_height; y++)
	  if (tile_data[y * 4 + x] &&
	      tetris->field[module->width * (tetris->y + y + 1) +
			                    (tetris->x + x) * 2] != 0)
	    {
	      stuck = TRUE;
	      break;
	    }
    }

  if (stuck)
    {
      if (tetris->y < 3)
	tetris->game_over = TRUE;

      if (tetris->speed > 100)
	tetris->speed -= ACC_ON_STUCK;

      for (x = 0; x < tile_width; x++)
	for (y = 0; y < tile_height; y++)
	  {
	    if (tile_data[y * 4 + x])
	      {
                guchar *data;

                data = (tetris->field +
                        module->width * (tetris->y + y) +
                        2 * (tetris->x + x));

		data[0] = tile[tetris->tile].color;
                data[1] = tile[tetris->tile].color;
	      }
	  }


      /* check for completed rows */
      for (y = module->height-BOTTOM_OFFSET-1; y > 0; y--)
	{
	  tetris->completed[y] = TRUE;
	  for (x = 0; x < module->width; x++)
	    {
	      if (tetris->field[module->width * y + x] == 0)
		tetris->completed[y] = FALSE;
	    }
	  if (tetris->completed[y])
	    tetris->n_completed++;
	}

      if (tetris->n_completed)
	tetris->n_completed = 5;

      /* select new tile and start over */
      tetris->y        = 0;
      tetris->x        = module->width / 4 - 1;
      tetris->rotation = 0;
      tetris->tile     = b_tetris_new_tile ();
    }
  else
    {
      tetris->y++;
    }

  b_tetris_paint (module);

  return stuck;
}

static void
b_tetris_left (BModule *module)
{
  BTetris     *tetris = B_TETRIS (module);
  const gchar *tile_data;
  gint         tile_width, tile_height;
  gint         x, y;

  tile_data   = tile[tetris->tile].data[tetris->rotation];
  tile_width  = b_tetris_tile_width  (tetris->tile, tetris->rotation);
  tile_height = b_tetris_tile_height (tetris->tile, tetris->rotation);

  if (tetris->x +
      b_tetris_tile_left_offset (tetris->tile, tetris->rotation) <= 0)
    return;

  for (x = 0; x < tile_width+1; x++)
    for (y = 0; y < tile_height; y++)
      if (tile_data[y * 4 + x] &&
	  tetris->field[module->width * (tetris->y + y) +
			(tetris->x + x) * 2 - 1] != 0)
	return;

  tetris->x--;
  b_tetris_paint (module);
}

static void
b_tetris_right (BModule *module)
{
  BTetris     *tetris = B_TETRIS (module);
  const gchar *tile_data;
  gint         tile_width, tile_height;
  gint         x, y;

  tile_data   = tile[tetris->tile].data[tetris->rotation];
  tile_width  = b_tetris_tile_width  (tetris->tile, tetris->rotation);
  tile_height = b_tetris_tile_height (tetris->tile, tetris->rotation);

  if (tetris->x * 2 >= (module->width -
			b_tetris_tile_width (tetris->tile,
					     tetris->rotation) * 2 - 1))
    return;

  for (x = 0; x < tile_width+1; x++)
    for (y = 0; y < tile_height; y++)
      if (tile_data[y * 4 + x] &&
	  tetris->field[module->width * (tetris->y + y) +
			(tetris->x + x + 1) * 2] != 0)
	return;

  tetris->x++;
  b_tetris_paint (module);
}

G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_tetris_get_type (module);
  return TRUE;
}

GType
b_tetris_get_type (GTypeModule *module)
{
  if (!b_type_tetris)
    {
      static const GTypeInfo tetris_info =
      {
        sizeof (BTetrisClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_tetris_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BTetris),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_tetris_init,
      };

      b_type_tetris = g_type_module_register_type (module,
                                                   B_TYPE_MODULE, "BTetris",
                                                   &tetris_info, 0);
    }

  return b_type_tetris;
}

static void
b_tetris_class_init (BTetrisClass *klass)
{
  BModuleClass *module_class;

  module_class = B_MODULE_CLASS (klass);

  module_class->max_players = 1;

  module_class->query    = b_tetris_query;
  module_class->prepare  = b_tetris_prepare;
  module_class->relax    = b_tetris_relax;
  module_class->start    = b_tetris_start;
  module_class->stop     = b_tetris_stop;
  module_class->event    = b_tetris_event;
  module_class->tick     = b_tetris_tick;
  module_class->describe = b_tetris_describe;
}

static void
b_tetris_init (BTetris *tetris)
{
  tetris->x                = 0;
  tetris->y                = 0;
  tetris->rotation         = 0;
  tetris->speed            = START_SPEED;
  tetris->tile             = -1;
  tetris->completed        = NULL;
  tetris->n_completed      = 0;
  tetris->player_device_id = -1;
}

static gboolean
b_tetris_query (gint     width,
                gint     height,
                gint     channels,
                gint     maxval)
{
#ifdef TETRIS_DEBUG
  g_print ("b_tetris_query\n");
#endif

  return (width > 8 && height > 6 && channels == 1 && maxval == 255);
}

static gboolean
b_tetris_prepare (BModule  *module,
                  GError  **error)
{
  BTetris *tetris = B_TETRIS (module);

#ifdef TETRIS_DEBUG
  g_print ("b_tetris_prepare\n");
#endif

  tetris->field_size = module->width * module->height;
  tetris->field      = g_new0 (guchar, tetris->field_size);
  tetris->completed  = g_new0 (gboolean, module->width);

  return TRUE;
}

static void
b_tetris_relax (BModule *module)
{
  BTetris *tetris = B_TETRIS (module);

#ifdef TETRIS_DEBUG
  g_print ("b_tetris_relax\n");
#endif

  if (tetris->field)
    {
      g_free (tetris->field);
      tetris->field = NULL;
    }

  if (tetris->completed)
    {
      g_free (tetris->completed);
      tetris->completed = NULL;
    }
}

static void
b_tetris_start (BModule *module)
{
  BTetris *tetris;

#ifdef TETRIS_DEBUG
  g_print ("b_tetris_start\n");
#endif

  tetris = B_TETRIS (module);

  tetris->x         = module->width / 4 - 1;
  tetris->y         = 0;
  tetris->rotation  = 0;
  tetris->tile      = b_tetris_new_tile ();
  tetris->game_over = FALSE;
  tetris->speed     = START_SPEED;

#ifdef TETRIS_DEBUG
  g_print (" tetris tile: %d\n", tetris->tile);
#endif

  memset (tetris->field, 0, tetris->field_size);
  memset (tetris->completed, 0, module->width);

  tetris->flash_state = FALSE;
  tetris->n_completed = 0;
  tetris->falling     = FALSE;

  b_module_fill (module, 0);
  b_module_ticker_start (module, 1000);
}

static void
b_tetris_stop (BModule *module)
{
  BTetris *tetris;

#ifdef TETRIS_DEBUG
  g_print ("b_tetris_stop\n");
#endif

  tetris = B_TETRIS (module);

  b_module_fill (module, 0);
  b_module_paint (module);
}

static void
b_tetris_event (BModule      *module,
                BModuleEvent *event)
{
  BTetris *tetris;

#ifdef TETRIS_DEBUG
  g_print ("b_tetris_event\n");
#endif

  tetris = B_TETRIS (module);
  if (tetris->falling)
    return;

  switch (event->type)
    {
    case B_EVENT_TYPE_KEY:
      if (event->device_id == tetris->player_device_id)
        {
          switch (event->key)
            {
            case B_KEY_4:
	      b_tetris_left (module);
              break;
            case B_KEY_5:
              b_tetris_rotate (module, TRUE);
              break;
            case B_KEY_6:
	      b_tetris_right (module);
              break;
            case B_KEY_7:
              b_tetris_rotate (module, TRUE);
              break;
            case B_KEY_8:
              if (!b_tetris_down (module))
                tetris->falling = TRUE;
              break;
            case B_KEY_9:
              b_tetris_rotate (module, FALSE);
              break;
            default:
              break;
            }
        }
      break;

    case B_EVENT_TYPE_PLAYER_ENTERED:
      if (tetris->player_device_id == -1)
        {
#ifdef TETRIS_DEBUG
	  g_print ("BTetris: player %d entered.\n", event->device_id);
#endif
          tetris->player_device_id = event->device_id;
          module->num_players++;
        }
      break;

    case B_EVENT_TYPE_PLAYER_LEFT:
      if (tetris->player_device_id == event->device_id)
        {
          tetris->player_device_id = -1;
          module->num_players--;
        }
      break;

    default:
      break;
    }
}

static gint
b_tetris_tick (BModule *module)
{
  BTetris *tetris;
  gint y, delta = 0;

#ifdef TETRIS_DEBUG
  g_print ("b_tetris_tick\n");
#endif
 
  tetris = B_TETRIS (module);

  if (tetris->game_over)
    {
      b_tetris_blend_down (module);
      b_module_request_stop (module);
      return 0;
    }

  if (tetris->falling)
    {
      if (b_tetris_down (module))
	{
	  tetris->falling = FALSE;
	}
      else
	return FALL_SPEED;
    }

  if (tetris->n_completed > 0)
    {
      /* flash completed rows */
      if (tetris->flash_state)
	{
	  memcpy (module->buffer, tetris->field, tetris->field_size);
	  b_module_paint (module);
	}
      else
	{
	  for (y = 0; y < module->height; y++)
	    if (tetris->completed[y])
	      memset (module->buffer + module->width * y, 0, module->width);

	  b_module_paint (module);

	  /* finally remove completed rows */
	  if (--tetris->n_completed == 0)
	    {
	      for (y = module->height - BOTTOM_OFFSET - 1; y >= 0; y--)
		if (tetris->completed[y - delta])
		  {
		    memmove (tetris->field + module->width,
			     tetris->field,
			     module->width * (y));
		    delta++;
		    y++;
		  }

	      memcpy (module->buffer, tetris->field, tetris->field_size);
	      b_module_paint (module);
	      memset (tetris->completed, 0, sizeof(gboolean) * module->height);

	      return tetris->speed;
	    }
	}

      tetris->flash_state = !(tetris->flash_state);

      return FLASH_SPEED;
    }

  if (b_tetris_down (module)) /* if stuck */
    return b_tetris_tick (module);

  return tetris->speed;
}

static void
b_tetris_describe (BModule      *module,
                   const gchar **title,
                   const gchar **description,
                   const gchar **author)
{
  *title       = "BTetris";
  *description = "Tetris game";
  *author      = "Daniel Mack";
}

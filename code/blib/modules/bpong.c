/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  Sven Neumann <sven@gimp.org>
 *                          Michael Natterer <mitch@gimp.org>
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

#include <glib-object.h>

#include <blib/blib.h>

#include "digits.h"


#define PONG_START_TIMEOUT      200
#define COMPUTER_LOOSE_SPEEDUP   10


typedef enum
{
  RIGHT,
  LEFT
} XDirection;

typedef enum
{
  UP,
  DOWN
} YDirection;


typedef enum
{
  GAME_INIT,
  LPLAYER_JOIN,
  RPLAYER_JOIN,
  GAME_OVER
} AnimType;


#define B_TYPE_PONG            (b_type_pong)
#define B_PONG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_PONG, BPong))
#define B_PONG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_PONG, BPongClass))
#define B_IS_PONG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_PONG))
#define B_IS_PONG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_PONG))

typedef struct _BPong      BPong;
typedef struct _BPongClass BPongClass;

struct _BPong
{
  BModule       parent_instance;

  AnimType      anim;
  gint          anim_steps;

  gint          paddle_size;

  gint          lpaddle;
  gint          rpaddle;
  gint          ball_x;
  gint          ball_y;
  XDirection    ball_xdir;
  YDirection    ball_ydir;

  gint          lplayer_score;
  gint          rplayer_score;

  gint          lplayer_device_id;
  gint          rplayer_device_id;

  gint          timeout;
};

struct _BPongClass
{
  BModuleClass  parent_class;
};


static GType      b_pong_get_type      (GTypeModule   *module);

static void       b_pong_class_init    (BPongClass    *klass);
static void       b_pong_init          (BPong         *pong);

static gboolean   b_pong_query         (gint           width,
                                        gint           height,
                                        gint           channels,
                                        gint           maxval);
static gboolean   b_pong_prepare       (BModule       *module,
                                        GError       **error);
static void       b_pong_relax         (BModule       *module);
static void       b_pong_start         (BModule       *module);
static void       b_pong_stop          (BModule       *module);
static void       b_pong_event         (BModule       *module,
                                        BModuleEvent  *event);
static gint       b_pong_tick          (BModule       *module);
static void       b_pong_describe      (BModule       *module,
                                        const gchar  **title,
                                        const gchar  **description,
                                        const gchar  **author);

static void       b_pong_init_game     (BPong         *pong);
static gint       b_pong_move_ball     (BPong         *pong);
static void       b_pong_computer_move (BPong         *pong,
                                        gint          *paddle);
static gboolean   b_pong_reflect       (BPong         *pong,
                                        gint           paddle);
static void       b_pong_draw          (BPong         *pong,
                                        gint           lpaddle,
                                        gint           rpaddle,
                                        gint           ball_x,
                                        gint           ball_y);
static void       b_pong_draw_scores   (BPong         *pong);


static GType b_type_pong = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_pong_get_type (module);
  return TRUE;
}

static GType
b_pong_get_type (GTypeModule *module)
{
  if (! b_type_pong)
    {
      static const GTypeInfo pong_info =
      {
        sizeof (BPongClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_pong_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BPong),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_pong_init,
      };

      b_type_pong = g_type_module_register_type (module,
                                                 B_TYPE_MODULE, "BPong",
                                                 &pong_info, 0);
    }

  return b_type_pong;
}

static void
b_pong_class_init (BPongClass *klass)
{
  BModuleClass *module_class;

  module_class = B_MODULE_CLASS (klass);

  module_class->max_players = 2;

  module_class->query    = b_pong_query;
  module_class->prepare  = b_pong_prepare;
  module_class->relax    = b_pong_relax;
  module_class->start    = b_pong_start;
  module_class->stop     = b_pong_stop;
  module_class->event    = b_pong_event;
  module_class->tick     = b_pong_tick;
  module_class->describe = b_pong_describe;
}

static void
b_pong_init (BPong *pong)
{
  pong->anim_steps        = 0;

  pong->lplayer_device_id = -1;
  pong->rplayer_device_id = -1;
}

static gboolean
b_pong_query (gint     width,
              gint     height,
              gint     channels,
              gint     maxval)
{
  return (width > 7 && height > 7 && channels == 1);
}

static gboolean
b_pong_prepare (BModule  *module,
                GError  **error)
{
  BPong *pong = B_PONG (module);

  pong->paddle_size = MAX (3, module->height / 4);

  return TRUE;
}

static void
b_pong_relax (BModule *module)
{
}

static void
b_pong_start (BModule *module)
{
  BPong *pong = B_PONG (module);

  pong->lpaddle       = pong->rpaddle       = module->height / 2 - 1;
  pong->lplayer_score = pong->rplayer_score = 0;

  b_pong_init_game (pong);

  pong->timeout = PONG_START_TIMEOUT;

  b_module_ticker_start (module, pong->timeout);
}

static void
b_pong_stop (BModule *module)
{
  BPong *pong = B_PONG (module);

  pong->lplayer_device_id = -1;
  pong->rplayer_device_id = -1;
}

static void
b_pong_event (BModule      *module,
              BModuleEvent *event)
{
  BPong *pong = B_PONG (module);

  switch (event->type)
    {
    case B_EVENT_TYPE_KEY:
      if (pong->anim_steps)
        return;

      switch (event->key)
        {
        case B_KEY_1:
        case B_KEY_2:
        case B_KEY_3:
        case B_KEY_4:
        case B_KEY_5:
        case B_KEY_6:
          /* up */
          if (event->device_id == pong->lplayer_device_id)
            {
              pong->lpaddle = MAX (pong->lpaddle--, 0);
            }
          else if (event->device_id == pong->rplayer_device_id)
            {
              pong->rpaddle = MAX (pong->rpaddle--, 0);
            }
          break;

        case B_KEY_7:
        case B_KEY_8:
        case B_KEY_9:
        case B_KEY_ASTERISK:
        case B_KEY_0:
        case B_KEY_HASH:
          /* down */
          if (event->device_id == pong->lplayer_device_id)
            {
              pong->lpaddle = MIN (pong->lpaddle++,
                                   module->height - pong->paddle_size);
            }
          else if (event->device_id == pong->rplayer_device_id)
            {
              pong->rpaddle = MIN (pong->rpaddle++,
                                   module->height - pong->paddle_size);
            }
          break;

        default:
          break;
        }
      break;

    case B_EVENT_TYPE_PLAYER_ENTERED:
      if (pong->lplayer_device_id == -1)
        {
          pong->anim              = LPLAYER_JOIN;
          pong->anim_steps        = 6;
          pong->lplayer_device_id = event->device_id;

          module->num_players++;
        }
      else if (pong->rplayer_device_id == -1)
        {
          pong->anim              = RPLAYER_JOIN;
          pong->anim_steps        = 6;
          pong->rplayer_device_id = event->device_id;

          module->num_players++;
        }
      break;

    case B_EVENT_TYPE_PLAYER_LEFT:
      if (pong->lplayer_device_id == event->device_id)
        {
          pong->lplayer_device_id = -1;

          module->num_players--;
        }
      else if (pong->rplayer_device_id == event->device_id)
        {
          pong->rplayer_device_id = -1;

          module->num_players--;
        }
      break;

    default:
      break;
    }
}

static gint
b_pong_tick (BModule *module)
{
  BPong *pong = B_PONG (module);

  if (pong->anim_steps > 0)
    {
      pong->anim_steps--;

      switch (pong->anim)
        {
        case GAME_INIT:
          b_pong_draw (pong,
                       pong->lpaddle, pong->rpaddle,
                       pong->anim_steps == 0 ? pong->ball_x : -1,
                       pong->anim_steps == 0 ? pong->ball_y : -1);
          break;
        case LPLAYER_JOIN:
          b_pong_draw (pong,
                       (pong->anim_steps & 1) ? pong->lpaddle : -1,
                       pong->rpaddle,
                       -1, -1);
          break;
        case RPLAYER_JOIN:
          b_pong_draw (pong,
                       pong->lpaddle,
                       (pong->anim_steps & 1) ? pong->rpaddle : -1,
                       -1, -1);
          break;
        case GAME_OVER:
          b_pong_draw (pong,
                       pong->lpaddle, pong->rpaddle,
                       (pong->anim_steps & 1) ? pong->ball_x : -1,
                       (pong->anim_steps & 1) ? pong->ball_y : -1);

          if (pong->anim_steps == 0)
            {
              if ((pong->lplayer_score != pong->rplayer_score) &&
                  (pong->lplayer_score >= 10 || pong->rplayer_score >= 10))
                {
                  b_module_request_stop (module);
                  return 0;
                }
              else
                b_pong_init_game (pong);
            }

          break;
        }
    }
  else
    {
      switch (b_pong_move_ball (pong))
        {
        case -1:
          pong->lplayer_score++;
          pong->anim_steps = 6;
          pong->anim = GAME_OVER;
	  if (pong->lplayer_device_id == -1) /* computer lost */
	    pong->timeout -= COMPUTER_LOOSE_SPEEDUP;
          break;

        case 1:
          pong->rplayer_score++;
          pong->anim_steps = 6;
          pong->anim = GAME_OVER;
	  if (pong->rplayer_device_id == -1) /* computer lost */
	    pong->timeout -= COMPUTER_LOOSE_SPEEDUP;
          break;

        case 0:
          if (pong->lplayer_device_id == -1)
            b_pong_computer_move (pong, &pong->lpaddle);
          if (pong->rplayer_device_id == -1)
            b_pong_computer_move (pong, &pong->rpaddle);
          break;
        }

      b_pong_draw (pong,
                   pong->lpaddle, pong->rpaddle,
                   pong->ball_x, pong->ball_y);
    }

  return pong->timeout;
}

static void
b_pong_describe (BModule      *module,
                 const gchar **title,
                 const gchar **description,
                 const gchar **author)
{
  *title       = "BPong";
  *description = "Pong game";
  *author      = "Sven Neumann";
}

static void
b_pong_init_game (BPong *pong)
{
  BModule *module;
  gint     paddle;
  gint     foo;

  module = B_MODULE (pong);

  foo = rand ();

  if (foo & 0x1)
    {
      paddle = pong->lpaddle;
      pong->ball_x    = 0;
      pong->ball_xdir = RIGHT;
    }
  else
    {
      paddle = pong->rpaddle;
      pong->ball_x    = module->width - 1;
      pong->ball_xdir = LEFT;
    }

  if ((foo & 0x2 && paddle != 0) ||
      paddle == module->height - pong->paddle_size)
    {
      pong->ball_y    = 0;
      pong->ball_ydir = DOWN;
    }
  else
    {
      pong->ball_y    = module->height - 1;
      pong->ball_ydir = UP;
    }

  pong->anim       = GAME_INIT;
  pong->anim_steps = 4;
}

static gint
b_pong_move_ball (BPong *pong)
{
  BModule *module = B_MODULE (pong);

  switch (pong->ball_xdir)
    {
    case RIGHT:
      pong->ball_x++;
      break;
    case LEFT:
      pong->ball_x--;
      break;
    }
  switch (pong->ball_ydir)
    {
    case UP:
      pong->ball_y--;
      break;
    case DOWN:
      pong->ball_y++;
      break;
    }

  /* collision with walls ? */
  if (pong->ball_y < 0)
    {
      pong->ball_y = 1;
      pong->ball_ydir = DOWN;
    }
  else if (pong->ball_y >= module->height)
    {
      pong->ball_y = module->height - 2;
      pong->ball_ydir = UP;
    }

  /* collision with left paddle or out ? */
  if (pong->ball_x <= 0)
    {
      if (! b_pong_reflect (pong, pong->lpaddle))
        return 1;  /* right wins */

      pong->ball_x = 2;
      pong->ball_xdir = RIGHT;
    }
  /* collision with right paddle or out ? */
  else if (pong->ball_x >= module->width - 1)
    {
      if (! b_pong_reflect (pong, pong->rpaddle))
        return -1;  /* left wins */
      pong->ball_x = module->width - pong->paddle_size;
      pong->ball_xdir = LEFT;
    }

  return 0;
}

static void
b_pong_computer_move (BPong *pong,
                      gint  *paddle)
{
  BModule *module = B_MODULE (pong);

  if (rand () & 1)
    return;

  if (*paddle - pong->ball_y > -1)
    (*paddle)--;
  else if (*paddle - pong->ball_y < 1)
    (*paddle)++;

  *paddle = CLAMP (*paddle, 0, module->height - pong->paddle_size);
}

static gboolean
b_pong_reflect (BPong *pong,
                gint   paddle)
{
  gint d = pong->ball_y - paddle;

  switch (pong->ball_ydir)
    {
    case DOWN: /* we hit the paddle coming from the top */

      if (d < 0)
        return FALSE;
      else if (d == 0)
        {
          pong->ball_ydir = UP;
          pong->ball_y -= 2;
          return TRUE;
        }
      else if (d <= pong->paddle_size)
        return TRUE;
      else
        return FALSE;

      break;

    case UP: /* we hit the paddle coming from the bottom */

      if (d < -1)
        return FALSE;
      else if (d < pong->paddle_size - 1)
        return TRUE;
      else if (d == pong->paddle_size - 1)
        {
          pong->ball_ydir = DOWN;
          pong->ball_y += 2;
          return TRUE;
        }
      else
        return FALSE;

      break;
    }

  return TRUE;
}

static void
b_pong_draw (BPong *pong,
             gint   lpaddle,
             gint   rpaddle,
             gint   ball_x,
             gint   ball_y)
{
  BModule *module;
  gint     width;
  gint     height;
  gint     i;

  module = B_MODULE (pong);

  width  = module->width;
  height = module->height;

  b_module_fill (module, 0);

  if (lpaddle >= 0 && lpaddle <= height - pong->paddle_size)
    for (i = 0; i < pong->paddle_size; i++)
      b_module_draw_point (module, 0, lpaddle + i, module->maxval);

  if (rpaddle >= 0 && rpaddle <= height - pong->paddle_size)
    for (i = 0; i < pong->paddle_size; i++)
      b_module_draw_point (module, width - 1, rpaddle + i, module->maxval);

  if (ball_x >= 0 && ball_x < width && ball_y >= 0 && ball_y < height)
    b_module_draw_point (module, ball_x, ball_y, module->maxval);

  if (pong->anim == GAME_OVER)
    b_pong_draw_scores (pong);

  b_module_paint (module);
}

static void
b_pong_draw_scores (BPong *pong)
{
  BModule     *module;
  gchar       *text;
  gint         len;
  gint         x0, y0;
  gint         x, y;
  gint         n, i;
  const BFont *digits = &b_digits_3x5;
   
 
  module = B_MODULE (pong);

  text = g_strdup_printf ("%d:%d", pong->lplayer_score, pong->rplayer_score);
  len = strlen (text);

  x0 = (module->width - len * digits->advance) / 2;
  y0 = (module->height / 2) - 4;

  for (n = 0; n < len; n++)
    {
      for (i = 0; i < digits->num_digits && digits->digits_str[i] != text[n]; i++);

      if (i < digits->num_digits)
        for (x = 0; x < digits->width; x++)
          for (y = 0; y < digits->height; y++)
            if (digits->data[i][y * digits->width + x] != '0')
              b_module_draw_point (module, x0 + x, y0 + y, module->maxval);

      x0 += digits->advance;
    }

  g_free (text);
}

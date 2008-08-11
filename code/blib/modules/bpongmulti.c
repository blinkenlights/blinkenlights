/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  Sven Neumann <sven@gimp.org>
 *                          Michael Natterer <mitch@gimp.org>
 * Changed: 2006 Stefan Schuermans <1stein@blinkenarea.org>
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


#define PONG_START_TIMEOUT       70
#define COMPUTER_LOOSE_SPEEDUP   3
#define BALL_CNT_MAX             5
#define BALL_INTERVAL10_MIN      20
#define BALL_INTERVAL10_MAX      40


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

typedef struct
{
  gint        x;
  gint        y;
  XDirection  xdir;
  YDirection  ydir;
  gint        interval10;   /* 1/10 inverse speed of ball (BALL_INTERVAL10_MIN..BALL_INTERVAL10_MAX) */
  gint        tick10;       /* 1/10 ticks since last move of ball */
} Ball;


#define B_TYPE_PONG            (b_type_pong)
#define B_PONG(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_PONG, BPongMulti))
#define B_PONG_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_PONG, BPongMultiClass))
#define B_IS_PONG(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_PONG))
#define B_IS_PONG_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_PONG))

typedef struct _BPongMulti      BPongMulti;
typedef struct _BPongMultiClass BPongMultiClass;

struct _BPongMulti
{
  BModule       parent_instance;

  AnimType      anim;
  gint          anim_steps;

  gint		ball_cnt;
  gint          paddle_size;

  gint          lpaddle;
  gint          rpaddle;
  Ball          balls[BALL_CNT_MAX];

  gint          lplayer_score;
  gint          rplayer_score;

  gint          lplayer_device_id;
  gint          rplayer_device_id;

  gint          timeout;
};

struct _BPongMultiClass
{
  BModuleClass  parent_class;
};


static GType      b_pong_multi_get_type      (GTypeModule   *module);

static void       b_pong_multi_class_init    (BPongMultiClass    *klass);
static void       b_pong_multi_init          (BPongMulti         *pong);

static gboolean   b_pong_multi_query         (gint           width,
                                              gint           height,
                                              gint           channels,
                                              gint           maxval);
static gboolean   b_pong_multi_prepare       (BModule       *module,
                                              GError       **error);
static void       b_pong_multi_relax         (BModule       *module);
static void       b_pong_multi_start         (BModule       *module);
static void       b_pong_multi_stop          (BModule       *module);
static void       b_pong_multi_event         (BModule       *module,
                                              BModuleEvent  *event);
static gint       b_pong_multi_tick          (BModule       *module);
static void       b_pong_multi_describe      (BModule       *module,
                                              const gchar  **title,
                                              const gchar  **description,
                                              const gchar  **author);

static void       b_pong_multi_init_game     (BPongMulti    *pong);
static gint       b_pong_multi_move_ball     (BPongMulti    *pong);
static void       b_pong_multi_computer_move (BPongMulti    *pong,
                                              gint          *paddle,
                                              gint          x);
static gboolean   b_pong_multi_reflect       (BPongMulti     *pong,
                                              gint           paddle,
                                              gint           ball_no);
static void       b_pong_multi_draw          (BPongMulti    *pong,
                                              gint           lpaddle,
                                              gint           rpaddle,
                                              gint           balls_draw_flags);
static void       b_pong_multi_draw_scores   (BPongMulti    *pong);


static GType b_type_pong = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule *module)
{
  b_pong_multi_get_type (module);
  return TRUE;
}

static GType
b_pong_multi_get_type (GTypeModule *module)
{
  if (! b_type_pong)
    {
      static const GTypeInfo pong_info =
      {
        sizeof (BPongMultiClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_pong_multi_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BPongMulti),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_pong_multi_init,
      };

      b_type_pong = g_type_module_register_type (module,
                                                 B_TYPE_MODULE, "BPongMulti",
                                                 &pong_info, 0);
    }

  return b_type_pong;
}

static void
b_pong_multi_class_init (BPongMultiClass *klass)
{
  BModuleClass *module_class;

  module_class = B_MODULE_CLASS (klass);

  module_class->max_players = 2;

  module_class->query    = b_pong_multi_query;
  module_class->prepare  = b_pong_multi_prepare;
  module_class->relax    = b_pong_multi_relax;
  module_class->start    = b_pong_multi_start;
  module_class->stop     = b_pong_multi_stop;
  module_class->event    = b_pong_multi_event;
  module_class->tick     = b_pong_multi_tick;
  module_class->describe = b_pong_multi_describe;
}

static void
b_pong_multi_init (BPongMulti *pong)
{
  pong->anim_steps        = 0;

  pong->lplayer_device_id = -1;
  pong->rplayer_device_id = -1;
}

static gboolean
b_pong_multi_query (gint     width,
                    gint     height,
                    gint     channels,
                    gint     maxval)
{
  return (width >= 7 && height >= 7 && channels == 1);
}

static gboolean
b_pong_multi_prepare (BModule  *module,
                      GError  **error)
{
  BPongMulti *pong = B_PONG (module);

  pong->ball_cnt    = MAX (1, MIN (module->width / 12, BALL_CNT_MAX));
  pong->paddle_size = MAX (3, module->height / 4);

  return TRUE;
}

static void
b_pong_multi_relax (BModule *module)
{
}

static void
b_pong_multi_start (BModule *module)
{
  BPongMulti *pong = B_PONG (module);

  pong->lpaddle       = pong->rpaddle       = module->height / 2 - 1;
  pong->lplayer_score = pong->rplayer_score = 0;

  b_pong_multi_init_game (pong);

  pong->timeout = PONG_START_TIMEOUT;

  b_module_ticker_start (module, pong->timeout);
}

static void
b_pong_multi_stop (BModule *module)
{
  BPongMulti *pong = B_PONG (module);

  pong->lplayer_device_id = -1;
  pong->rplayer_device_id = -1;
}

static void
b_pong_multi_event (BModule      *module,
                    BModuleEvent *event)
{
  BPongMulti *pong = B_PONG (module);

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
          pong->anim_steps        = 18;
          pong->lplayer_device_id = event->device_id;

          module->num_players++;
        }
      else if (pong->rplayer_device_id == -1)
        {
          pong->anim              = RPLAYER_JOIN;
          pong->anim_steps        = 18;
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
b_pong_multi_tick (BModule *module)
{
  BPongMulti *pong = B_PONG (module);

  if (pong->anim_steps > 0)
    {
      pong->anim_steps--;

      switch (pong->anim)
        {
        case GAME_INIT:
          b_pong_multi_draw (pong,
                       pong->lpaddle, pong->rpaddle,
                       pong->anim_steps < 3 ? -1 : 0);
          break;
        case LPLAYER_JOIN:
          b_pong_multi_draw (pong,
                       (pong->anim_steps / 3 & 1) ? pong->lpaddle : -1,
                       pong->rpaddle,
                       0);
          break;
        case RPLAYER_JOIN:
          b_pong_multi_draw (pong,
                       pong->lpaddle,
                       (pong->anim_steps / 3 & 1) ? pong->rpaddle : -1,
                       0);
          break;
        case GAME_OVER:
          b_pong_multi_draw (pong,
                       pong->lpaddle, pong->rpaddle,
                       (pong->anim_steps / 3 & 1) ? -1 : 0);

          if (pong->anim_steps == 0)
            {
              if ((pong->lplayer_score != pong->rplayer_score) &&
                  (pong->lplayer_score >= 10 || pong->rplayer_score >= 10))
                {
                  b_module_request_stop (module);
                  return 0;
                }
              else
                b_pong_multi_init_game (pong);
            }

          break;
        }
    }
  else
    {
      switch (b_pong_multi_move_ball (pong))
        {
        case 1: /* left wins */
          pong->lplayer_score++;
          pong->anim_steps = 18;
          pong->anim = GAME_OVER;
	  if (pong->lplayer_device_id == -1) /* computer lost */
	    pong->timeout -= COMPUTER_LOOSE_SPEEDUP;
          break;

        case 2: /* right wins*/
          pong->rplayer_score++;
          pong->anim_steps = 18;
          pong->anim = GAME_OVER;
	  if (pong->rplayer_device_id == -1) /* computer lost */
	    pong->timeout -= COMPUTER_LOOSE_SPEEDUP;
          break;

        case -1: /* stalemate */
          pong->anim_steps = 18;
          pong->anim = GAME_OVER;
          break;

        case 0: /* game goes on */
          if (pong->lplayer_device_id == -1)
            b_pong_multi_computer_move (pong, &pong->lpaddle, 0);
          if (pong->rplayer_device_id == -1)
            b_pong_multi_computer_move (pong, &pong->rpaddle, module->width - 1);
          break;
        }

      b_pong_multi_draw (pong,
                   pong->lpaddle, pong->rpaddle,
                   -1);
    }

  return pong->timeout;
}

static void
b_pong_multi_describe (BModule      *module,
                 const gchar **title,
                 const gchar **description,
                 const gchar **author)
{
  *title       = "BPongMulti";
  *description = "Pong game";
  *author      = "Sven Neumann";
}

static void
b_pong_multi_init_game (BPongMulti *pong)
{
  BModule *module;
  gint     paddle;
  gint     foo;
  gint     i;

  module = B_MODULE (pong);

  for (i = 0; i < pong->ball_cnt; i++)  {
    foo = rand ();

    if (foo & 0x1) {
      paddle = pong->lpaddle;
      pong->balls[i].x    = 0;
      pong->balls[i].xdir = RIGHT;
    } else {
    paddle = pong->rpaddle;
      pong->balls[i].x    = module->width - 1;
      pong->balls[i].xdir = LEFT;
    }

    if ((foo & 0x2 && paddle != 0) ||
        paddle == module->height - pong->paddle_size)
    {
      pong->balls[i].y    = 0;
      pong->balls[i].ydir = DOWN;
    } else {
      pong->balls[i].y    = module->height - 1;
      pong->balls[i].ydir = UP;
    }

    pong->balls[i].interval10 = rand () % (BALL_INTERVAL10_MAX - BALL_INTERVAL10_MIN + 1) + BALL_INTERVAL10_MIN;
    for (;;)  {
      gint j; /* check for balls with same speed */
      for (j = 0; j < i; j++)
        if (pong->balls[j].interval10 == pong->balls[i].interval10)
          break;
      if (j >= i)  /* no other ball has same speed */
        break;
      pong->balls[i].interval10++; /* modify speed */
      if (pong->balls[i].interval10 >= BALL_INTERVAL10_MAX)
        pong->balls[i].interval10 = BALL_INTERVAL10_MIN;
    }
    pong->balls[i].tick10 = 0;
  }

  pong->anim       = GAME_INIT;
  pong->anim_steps = 12;
}

static gint
b_pong_multi_move_ball (BPongMulti *pong)
{
  gint b;
  BModule *module = B_MODULE (pong);
  gint win_left = 0;
  gint win_right = 0;

  for (b = 0; b < pong->ball_cnt; b++)  {
    /* check if ball is to move now */
    pong->balls[b].tick10 += 10;
    if (pong->balls[b].tick10 >= pong->balls[b].interval10)  {
      pong->balls[b].tick10 -= pong->balls[b].interval10;

      switch (pong->balls[b].xdir)
        {
        case RIGHT:
          pong->balls[b].x++;
          break;
        case LEFT:
          pong->balls[b].x--;
          break;
        }
      switch (pong->balls[b].ydir)
        {
        case UP:
          pong->balls[b].y--;
          break;
        case DOWN:
          pong->balls[b].y++;
          break;
        }

      /* collision with walls ? */
      if (pong->balls[b].y < 0)
        {
          pong->balls[b].y = 1;
          pong->balls[b].ydir = DOWN;
        }
      else if (pong->balls[b].y >= module->height)
        {
          pong->balls[b].y = module->height - 2;
          pong->balls[b].ydir = UP;
        }

      /* collision with left paddle or out ? */
      if (pong->balls[b].x <= 0)
        {
          if (! b_pong_multi_reflect (pong, pong->lpaddle, b))
            win_right++;  /* right wins */

          pong->balls[b].x = 2;
          pong->balls[b].xdir = RIGHT;
        }
      /* collision with right paddle or out ? */
      else if (pong->balls[b].x >= module->width - 1)
        {
          if (! b_pong_multi_reflect (pong, pong->rpaddle, b))
            win_left++;  /* left wins */

          pong->balls[b].x = module->width - pong->paddle_size;
          pong->balls[b].xdir = LEFT;
        }

    }
  }

  if (win_left + win_right > 0)  {
    if (win_left > win_right)
      return 1; /* left wins */
    if (win_right > win_left)
      return 2; /* right wins */
    return -1; /* stalemate */
  }
  return 0;
}

static void
b_pong_multi_computer_move (BPongMulti *pong,
                            gint  *paddle,
                            gint  x)
{
  BModule *module = B_MODULE (pong);
  gint ball_no, b, dx, dx_min;

  /* computer is not perfect - it forgets to move often enough */
  if (rand () & 7)
    return;

  /* determine number of nearest ball */
  dx_min = module->width << 1;
  ball_no = 0;
  for (b = 0; b < pong->ball_cnt; b++)  {
    dx = pong->balls[b].x - x; /* get distance */
    if (dx < 0)
      dx = -dx;
    if (dx < dx_min)  { /* ball is nearer */
      ball_no = b;
      dx_min = dx;
    }
  }

  /* move computer towards nearest ball */
  if (*paddle - pong->balls[ball_no].y > -1)
    (*paddle)--;
  else if (*paddle - pong->balls[ball_no].y < 1)
    (*paddle)++;

  *paddle = CLAMP (*paddle, 0, module->height - pong->paddle_size);
}

static gboolean
b_pong_multi_reflect (BPongMulti *pong,
                      gint   paddle,
                      gint   ball_no)
{
  gint d = pong->balls[ball_no].y - paddle;

  switch (pong->balls[ball_no].ydir)
    {
    case DOWN: /* we hit the paddle coming from the top */

      if (d < 0)
        return FALSE;
      else if (d == 0)
        {
          pong->balls[ball_no].ydir = UP;
          pong->balls[ball_no].y -= 2;
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
          pong->balls[ball_no].ydir = DOWN;
          pong->balls[ball_no].y += 2;
          return TRUE;
        }
      else
        return FALSE;

      break;
    }

  return TRUE;
}

static void
b_pong_multi_draw (BPongMulti *pong,
             gint   lpaddle,
             gint   rpaddle,
             gint   balls_draw_flags)
{
  BModule *module;
  gint     width;
  gint     height;
  gint     i;
  gint     ball_x;
  gint     ball_y;

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

  for (i = 0; i < pong->ball_cnt; i++)  {
    ball_x = balls_draw_flags & (1 << i) ? pong->balls[i].x : -1;
    ball_y = balls_draw_flags & (1 << i) ? pong->balls[i].y : -1;
    if (ball_x >= 0 && ball_x < width && ball_y >= 0 && ball_y < height)
      b_module_draw_point (module, ball_x, ball_y, module->maxval);
  }

  if (pong->anim == GAME_OVER)
    b_pong_multi_draw_scores (pong);

  b_module_paint (module);
}

static void
b_pong_multi_draw_scores (BPongMulti *pong)
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
  y0 = (module->height / 2) - 3;

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

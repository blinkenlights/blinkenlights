/* blccc - BlinkenLigths Chaos Control Center
 *
 * Copyright (c) 2001  Sven Neumann <sven@gimp.org>
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

#include <gtk/gtk.h>

#include "bltypes.h"
#include "blccc.h"
#include "blisdn.h"
#include "blpong.h"


static void      bl_pong_class_init    (BlPongClass *class);
static void      bl_pong_init          (BlPong      *pong);
static void      bl_pong_destroy       (GtkObject   *object);
static void      bl_pong_start         (BlPong      *pong);
static void      bl_pong_stop          (BlPong      *pong);


enum 
{
  STARTED,
  FINISHED,
  NEW_FRAME,
  LAST_SIGNAL
};
static guint bl_pong_signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;


GtkType
bl_pong_get_type (void)
{
  static GtkType pong_type = 0;

  if (!pong_type)
    {
      GtkTypeInfo pong_info =
      {
	"BlPong",
	sizeof (BlPong),
	sizeof (BlPongClass),
	(GtkClassInitFunc) bl_pong_class_init,
	(GtkObjectInitFunc) bl_pong_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL
      };

      pong_type = gtk_type_unique (gtk_object_get_type (), &pong_info);
    }
  
  return pong_type;
}

static void
bl_pong_class_init (BlPongClass *class)
{
  GtkObjectClass *object_class;

  parent_class = gtk_type_class (gtk_object_get_type ());
  object_class = GTK_OBJECT_CLASS (class);

  bl_pong_signals[STARTED] = 
    gtk_signal_new ("started",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (BlPongClass, started),
		    gtk_signal_default_marshaller, 
                    GTK_TYPE_NONE, 0);
  bl_pong_signals[FINISHED] = 
    gtk_signal_new ("finished",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (BlPongClass, finished),
		    gtk_signal_default_marshaller, 
                    GTK_TYPE_NONE, 0);
  bl_pong_signals[NEW_FRAME] = 
    gtk_signal_new ("new_frame",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (BlPongClass, new_frame),
		    gtk_signal_default_marshaller, 
                    GTK_TYPE_NONE, 0);

  class->started   = NULL;
  class->finished  = NULL;
  class->new_frame = NULL;

  gtk_object_class_add_signals (object_class, bl_pong_signals, LAST_SIGNAL);

  object_class->destroy = bl_pong_destroy;
}

static void
bl_pong_init (BlPong *pong)
{
  pong->isdn          = NULL;
  pong->width         = 0;
  pong->height        = 0;

  pong->timeout_id    = 0;
  pong->anim_steps    = 0;

  pong->lplayer_human = FALSE;
  pong->rplayer_human = FALSE;
}

static void
bl_pong_destroy (GtkObject *object)
{
  BlPong *pong;

  pong = BL_PONG (object);

  if (pong->isdn)
    {
      gtk_object_unref (GTK_OBJECT (pong->isdn));
      pong->isdn = NULL;
    }

  if (pong->matrix)
    {
      g_free (pong->matrix);
      pong->matrix = NULL;
    }

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static void
bl_pong_init_game (BlPong *pong)
{
  gint foo;

  pong->lpaddle = pong->height / 2 - 1;
  pong->rpaddle = pong->height / 2 - 1;  
  
  foo = rand();

  if (foo & 0x1)
    {
      pong->ball_x = 0;
      pong->ball_xdir = RIGHT;
    }
  else
    {
      pong->ball_x = pong->width - 1;
      pong->ball_xdir = LEFT;
    }

  if (foo & 0x2)
    {
      pong->ball_y = 0;
      pong->ball_ydir = DOWN;
    }
  else
    {
      pong->ball_y = pong->height - 1;
      pong->ball_ydir = UP;
    }

  pong->anim = GAME_INIT;
  pong->anim_steps = 4;
}

static void
bl_pong_change_players (BlPong   *pong,
                        gboolean  lplayer_human,
                        gboolean  rplayer_human)
{
  if (pong->lplayer_human != lplayer_human)
    {
      pong->anim = LPLAYER_JOIN;
      pong->anim_steps = 6;
    }
  if (pong->rplayer_human != rplayer_human)
    {
      pong->anim = RPLAYER_JOIN;
      pong->anim_steps = 6;
    }
  
  pong->lplayer_human = lplayer_human;
  pong->rplayer_human = rplayer_human;
}

static gboolean
reflect (BlPong *pong,
         gint    paddle)
{
  switch (pong->ball_ydir)
    {
    case DOWN: /* we hit the paddle coming from the top */
      switch (pong->ball_y - paddle)
        {
        case 0:
          pong->ball_ydir = UP;
          pong->ball_y -= 2;
          break;
        case 1:
        case 2:
        case 3:
          break;
        default:
          return FALSE;
        }
      break;

    case UP: /* we hit the paddle coming from the bottom */
      switch (pong->ball_y - paddle)
        {
        case -1:
        case 0:
        case 1:
          break;
        case 2:
          pong->ball_ydir = DOWN;
          pong->ball_y += 2;
          break;
        default:
          return FALSE;
        }
      break;          
    }

  return TRUE;
}

static gint
move_ball (BlPong *pong)
{
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
  else if (pong->ball_y >= pong->height)
    {
      pong->ball_y = pong->height - 2;
      pong->ball_ydir = UP;
    }
  
  /* collision with left paddle or out ? */
  if (pong->ball_x == 0)
    {
      if (!reflect (pong, pong->lpaddle))
        return 1;  /* right wins */

      pong->ball_x = 2;
      pong->ball_xdir = RIGHT;
    }
  /* collision with right paddle or out ? */
  else if (pong->ball_x == pong->width - 1)
    {
      if (!reflect (pong, pong->rpaddle))
        return -1;  /* left wins */
      pong->ball_x = pong->width - 3;
      pong->ball_xdir = LEFT;
    }

  return 0;
}

static void
computer_move (BlPong *pong,
               gint   *paddle)
{
  if (rand() & 1)
    return;

  if (*paddle - pong->ball_y > -1)
    (*paddle)--;
  else if (*paddle - pong->ball_y < 1)
    (*paddle)++;
  
  *paddle = CLAMP (*paddle, 0, pong->height - 3);
}

static void
isdn_state_changed (BlIsdn *isdn,
                    BlPong *pong)
{
  if (pong->timeout_id)
    {
      if (!isdn->line0_offhook && !isdn->line1_offhook)
        {
          bl_pong_stop (pong);
        }
      else
        {
          bl_pong_change_players (pong, 
                                  isdn->line0_offhook, isdn->line1_offhook);
        }
    }
  else
    {
      if (isdn->line0_offhook || isdn->line1_offhook)
        {
          bl_pong_init_game (pong);
          bl_pong_change_players (pong, 
                                  isdn->line0_offhook, isdn->line1_offhook);
          bl_pong_start (pong);
        }
    }
}

static void
move_by_tone (gint *paddle,
              gint  height,
              gint  tone)
{
  switch (tone)
    {
    case 0x0:
    case 0xFF:
      break;

    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
      *paddle = MAX ((*paddle)--, 0);
      break;

    case '7':
    case '8':
    case '9':
    case '*':
    case '0':
    case '#':
      *paddle = MIN ((*paddle)++, height - 3);
      break;

    default:
      g_warning ("Unhandled dialtone: %c", tone);
      break;
    }
}

static void
isdn_dial_tone (BlIsdn *isdn,
                gint    tone0,
                gint    tone1,
                BlPong *pong)
{
  if (pong->anim_steps)
    return;

  if (pong->lplayer_human)
    move_by_tone (&pong->lpaddle, pong->height, tone0);

  if (pong->rplayer_human)
    move_by_tone (&pong->rpaddle, pong->height, tone1);
}

static void
bl_pong_draw (BlPong *pong,
              gint    lpaddle,
              gint    rpaddle,
              gint    ball_x,
              gint    ball_y)
{
  gint width  = pong->width;
  gint height = pong->height;
 
  memset (pong->matrix, 0, width * height);

  if (lpaddle >= 0 && lpaddle < height - 2)
    {
      lpaddle = lpaddle * width;
      pong->matrix[lpaddle]             = 1;
      pong->matrix[lpaddle +     width] = 1;
      pong->matrix[lpaddle + 2 * width] = 1;
    }
  
  if (rpaddle >= 0 && rpaddle < height - 2)
    {
      rpaddle = rpaddle * width + (width - 1);
      pong->matrix[rpaddle]             = 1;
      pong->matrix[rpaddle +     width] = 1;
      pong->matrix[rpaddle + 2 * width] = 1;
    }

  if (ball_x >= 0 && ball_x < width && ball_y >= 0 && ball_y < height)
    pong->matrix[ball_y * width + ball_x] = 1;

  gtk_signal_emit (GTK_OBJECT (pong), bl_pong_signals[NEW_FRAME], NULL);
}

static gboolean
bl_pong_timeout (BlPong *pong)
{
  if (pong->anim_steps > 0)
    {
      pong->anim_steps--;

      switch (pong->anim)
        {
        case GAME_INIT:
          bl_pong_draw (pong, 
                        pong->lpaddle, pong->rpaddle, 
                        pong->anim_steps == 0 ? pong->ball_x : -1, 
                        pong->anim_steps == 0 ? pong->ball_y : -1);
          break;
        case LPLAYER_JOIN:
          bl_pong_draw (pong, 
                        (pong->anim_steps & 1) ? pong->lpaddle : -1, 
                        pong->rpaddle, 
                        -1, -1);
          break;
        case RPLAYER_JOIN:
          bl_pong_draw (pong, 
                        pong->lpaddle, 
                        (pong->anim_steps & 1) ? pong->rpaddle : -1, 
                        -1, -1);
          break;
        case GAME_OVER:
          bl_pong_draw (pong, 
                        pong->lpaddle, pong->rpaddle, 
                        (pong->anim_steps & 1) ? pong->ball_x : -1,
                        (pong->anim_steps & 1) ? pong->ball_y : -1);

          if (pong->anim_steps == 0)
            bl_pong_init_game (pong);
          break;
        }
    }
  else
    {
      if (move_ball (pong))
        {
          pong->anim_steps = 6;
          pong->anim = GAME_OVER;
        }
      else
        {
          if (!pong->lplayer_human)
            computer_move (pong, &pong->lpaddle);
          if (!pong->rplayer_human)
            computer_move (pong, &pong->rpaddle);
        }

      bl_pong_draw (pong, 
                    pong->lpaddle, pong->rpaddle,
                    pong->ball_x, pong->ball_y);
    }

  return TRUE;
}

static void
bl_pong_start (BlPong *pong)
{
  g_return_if_fail (pong != NULL);
  g_return_if_fail (BL_IS_PONG (pong));

  if (pong->timeout_id == 0)
    {
      gtk_signal_emit (GTK_OBJECT (pong), bl_pong_signals[STARTED], NULL);

      pong->timeout_id = g_timeout_add (PONG_TIMEOUT,
                                        (GSourceFunc) bl_pong_timeout,
                                        pong);
    }
}

static void
bl_pong_stop (BlPong *pong)
{
  g_return_if_fail (pong != NULL);
  g_return_if_fail (BL_IS_PONG (pong));

  if (pong->timeout_id)
    {
      g_source_remove (pong->timeout_id);
      pong->timeout_id = 0;
      
      pong->lplayer_human = FALSE;
      pong->rplayer_human = FALSE;

      gtk_signal_emit (GTK_OBJECT (pong), bl_pong_signals[FINISHED], NULL);
    }
}

BlPong * 
bl_pong_new (BlIsdn *isdn,
             gint    width,
             gint    height)
{
  BlPong *pong;

  g_return_val_if_fail (isdn != NULL, NULL);
  g_return_val_if_fail (BL_IS_ISDN (isdn), NULL);  

  g_return_val_if_fail (width > 3 && height > 3, NULL);

  pong = BL_PONG (gtk_object_new (BL_TYPE_PONG, NULL));

  pong->width  = width;
  pong->height = height;

  pong->matrix = g_new0 (guchar, width * height);

  if (isdn)
    {
      gtk_object_ref (GTK_OBJECT (isdn));
      gtk_object_sink (GTK_OBJECT (isdn));
      pong->isdn = isdn;

      gtk_signal_connect (GTK_OBJECT (isdn), "state_changed", 
                          GTK_SIGNAL_FUNC (isdn_state_changed), 
                          pong);
      gtk_signal_connect (GTK_OBJECT (isdn), "dial_tone", 
                          GTK_SIGNAL_FUNC (isdn_dial_tone), 
                          pong);
    }

  return pong;
}

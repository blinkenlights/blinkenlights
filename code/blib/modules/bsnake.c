/* BSnake: Snake module for Blinkenlights
 *
 * Copyright (c) 2003  1stein <1stein@schuermans.info>
 *
 * based on Test implementation for a BModule by the Blinkenlights Crew
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

#include "config.h"

#include <string.h>
#include <stdlib.h>

#include <glib-object.h>
#include <gmodule.h>

#include <blib/blib.h>


#define BSnakeVerMaj 1
#define BSnakeVerMin 1
#define BSnakeVerRev 2
#define BSnakeVerTxt "1.1.2"

/* minimum size and color count of display */
#define BSnakeSizeXMin 18
#define BSnakeSizeYMin 8
#define BSnakeMaxColorMin 1
/* initial and maximum length of the snake */
#define BSnakeLenIni(Width, Height) 3
#define BSnakeLenMax(Width, Height) ((Width) + (Height) + 3)
/* time snake is dead after chrashing before game ends */
#define BSnakeDeadTime 10
#define BSnakeLives 3
/* initial and maximum speed of game (high number = slow) */
#define BSnakeSpeedIni 500
#define BSnakeSpeedMax 100
/* the colors */
#define BSnakeColorEmpty(MaxColor)     (0)
#define BSnakeColorSnakeHead(MaxColor) (MaxColor)
#define BSnakeColorSnakeBody(MaxColor) ((MaxColor) * 2 / 3)
#define BSnakeColorMouse(MaxColor)     ((MaxColor) * 3 / 4)

#ifdef SNAKE_DEBUG
#define dbg_print g_print
#else
#define dbg_print(fmt, ...)
#endif

#define B_TYPE_SNAKE_MODULE         (b_type_snake_module)
#define B_SNAKE_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_SNAKE_MODULE, BSnakeModule))
#define B_SNAKE_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_SNAKE_MODULE, BSnakeModuleClass))
#define B_IS_SNAKE_MODULE(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_SNAKE_MODULE))

typedef struct _BSnakeModule BSnakeModule;
typedef struct _BSnakeModuleClass BSnakeModuleClass;

typedef struct sBSnakePoint
{
  gint X, Y;
} stBSnakePoint;

struct _BSnakeModule
{
  BModule parent_instance;

  gint MaxColor;              /* maximum color value */
  stBSnakePoint Size;         /* size of game-field */
  gint SnakeLenIni;           /* initial size of the snake */
  gint SnakeLenMax;           /* maximum size of the snake */
  gint PlayerId;              /* device id of the player */
  gint SnakeLen;              /* current length of the snake */
  stBSnakePoint * pSnake;     /* array with fields of the snake (index 0 is head) */
  stBSnakePoint SnakeDir;     /* current direction of snake head */
  stBSnakePoint SnakeLastDir; /* direction of the last step made by the snake */
  stBSnakePoint Mouse;        /* position of the current mouse to eat ({-1,-1} if no mouse) */
  gint Speed;                 /* current speed of game */
  gint DeadCnt;               /* if > 0: snake has been dead for DeadCnt ticks */
  gint Lives;                 /* number of lives left */
};

struct _BSnakeModuleClass
{
  BModuleClass parent_class;
};

static GType    b_snake_module_get_type   (GTypeModule         *module);
static void     b_snake_module_class_init (BSnakeModuleClass   *klass);
static void     b_snake_module_init       (BSnakeModule        *test_module);
static gboolean b_snake_module_query      (gint                 width,
                                           gint                 height,
                                           gint                 channels,
                                           gint                 maxval);
static gboolean b_snake_module_prepare    (BModule             *module,
                                           GError             **error);
static void     b_snake_module_relax      (BModule             *module);
static void     b_snake_module_start      (BModule             *module);
static void     b_snake_module_event      (BModule             *module,
                                           BModuleEvent        *event);
static gint     b_snake_module_tick       (BModule             *module);
static void     b_snake_module_describe   (BModule             *module,
                                           const gchar        **title,
                                           const gchar        **description,
                                           const gchar        **author);


static GType b_type_snake_module = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule * module)
{
  b_snake_module_get_type (module);

  return TRUE;
}

GType
b_snake_module_get_type (GTypeModule * module)
{
  if (!b_type_snake_module)
    {
      static const GTypeInfo snake_module_info = {
        sizeof (BSnakeModuleClass),
        NULL,                        /* base_init */
        NULL,                        /* base_finalize */
        (GClassInitFunc) b_snake_module_class_init,
        NULL,                        /* class_finalize */
        NULL,                        /* class_data */
        sizeof (BSnakeModule),
        0,                        /* n_preallocs */
        (GInstanceInitFunc) b_snake_module_init,
      };

      /* !!!!!!!!! The name given in the next function MUST be unique! */

      b_type_snake_module = g_type_module_register_type (module,
                                                         B_TYPE_MODULE,
                                                         "BSnake",
                                                         &snake_module_info,
                                                         0);
    }

  return b_type_snake_module;
}

/* check if two points are equal */
static gint BSnakePointEqual (stBSnakePoint P1, stBSnakePoint P2)
{
  return P1.X == P2.X && P1.Y == P2.Y;
}

/* check if two points are near to each other (euclidian distance < 2) */
static gint BSnakePointNear (stBSnakePoint P1, stBSnakePoint P2)
{
  stBSnakePoint Delta;
  Delta.X = P2.X - P1.X;
  Delta.Y = P2.Y - P1.Y;
  return Delta.X >= -1 && Delta.X <= 1 && Delta.Y >= -1 && Delta.Y <= 1;
}

/* output current picture */
static void
BSnakeOutput (BSnakeModule *pSnakeModule)
{
  gint Color, I;

  /* empty the screen */
  b_module_fill ((BModule *) pSnakeModule,
                 BSnakeColorEmpty (pSnakeModule->MaxColor));

  /* output nothing if not almost end of game */
  if (pSnakeModule->DeadCnt < BSnakeDeadTime)
  {
    /* let snake blink when dead */
    if ((pSnakeModule->DeadCnt & 1) == 0)
    {
      /* draw snake */
      if (pSnakeModule->pSnake[0].X >= 0 /* snake head */
       && pSnakeModule->pSnake[0].X < pSnakeModule->Size.X
       && pSnakeModule->pSnake[0].Y >= 0
       && pSnakeModule->pSnake[0].Y < pSnakeModule->Size.Y)
        b_module_draw_point ((BModule *) pSnakeModule,
                             pSnakeModule->pSnake[0].X,
                             pSnakeModule->pSnake[0].Y,
                             BSnakeColorSnakeHead (pSnakeModule->MaxColor));
      Color = BSnakeColorSnakeBody (pSnakeModule->MaxColor); /* snake body */
      for (I = 1; I < pSnakeModule->SnakeLen; I++)
        if (pSnakeModule->pSnake[I].X >= 0
         && pSnakeModule->pSnake[I].X < pSnakeModule->Size.X
         && pSnakeModule->pSnake[I].Y >= 0
         && pSnakeModule->pSnake[I].Y < pSnakeModule->Size.Y)
            b_module_draw_point ((BModule *) pSnakeModule,
                                 pSnakeModule->pSnake[I].X,
                                 pSnakeModule->pSnake[I].Y,
                                 Color);
    }

    /* draw mouse */
    if (pSnakeModule->Mouse.X >= 0
     && pSnakeModule->Mouse.X < pSnakeModule->Size.X
     && pSnakeModule->Mouse.Y >= 0
     && pSnakeModule->Mouse.Y < pSnakeModule->Size.Y)
      b_module_draw_point ((BModule *) pSnakeModule,
                           pSnakeModule->Mouse.X,
                           pSnakeModule->Mouse.Y,
                           BSnakeColorMouse (pSnakeModule->MaxColor));
  }

  /* update screen */
  b_module_paint ((BModule *) pSnakeModule);
}

/* start a new game */
static void
BSnakeNewGame (BSnakeModule *pSnakeModule)
{
  gint I;

  /* initialize snake */
  pSnakeModule->SnakeLen = pSnakeModule->SnakeLenIni; /* length of snake to initial size */
  switch (rand () % 4)  /* place snake head and set direction */
  {
    case 0: /* snake comes from top */
      pSnakeModule->SnakeDir.X = 0;
      pSnakeModule->SnakeDir.Y = 1;
      pSnakeModule->pSnake[0].X = rand () % pSnakeModule->Size.X;
      pSnakeModule->pSnake[0].Y = 0;
      break;
    case 1: /* snake comes from left */
      pSnakeModule->SnakeDir.X = 1;
      pSnakeModule->SnakeDir.Y = 0;
      pSnakeModule->pSnake[0].X = 0;
      pSnakeModule->pSnake[0].Y = rand () % pSnakeModule->Size.Y;
      break;
    case 2: /* snake comes from right */
      pSnakeModule->SnakeDir.X = -1;
      pSnakeModule->SnakeDir.Y = 0;
      pSnakeModule->pSnake[0].X = pSnakeModule->Size.X - 1;
      pSnakeModule->pSnake[0].Y = rand () % pSnakeModule->Size.Y;
      break;
    case 3: /* snake comes from bottom */
      pSnakeModule->SnakeDir.X = 0;
      pSnakeModule->SnakeDir.Y = -1;
      pSnakeModule->pSnake[0].X = rand () % pSnakeModule->Size.X;
      pSnakeModule->pSnake[0].Y = pSnakeModule->Size.Y - 1;
      break;
  }
  for (I = 1; I < pSnakeModule->SnakeLen; I++)  /* initialize body of snake */
  {
    pSnakeModule->pSnake[I].X = pSnakeModule->pSnake[I-1].X
                              - pSnakeModule->SnakeDir.X;
    pSnakeModule->pSnake[I].Y = pSnakeModule->pSnake[I-1].Y
                              - pSnakeModule->SnakeDir.Y;
  }
  pSnakeModule->SnakeLastDir = pSnakeModule->SnakeDir; /* last step was in same direction */

  /* no mouse yet */
  pSnakeModule->Mouse.X = -1;
  pSnakeModule->Mouse.Y = -1;

  /* set speed to initial value */
  pSnakeModule->Speed = BSnakeSpeedIni;

  /* snake is alive */
  pSnakeModule->DeadCnt = 0;

  dbg_print ("BSnake: new game\n");

  /* output current picture */
  BSnakeOutput (pSnakeModule);
}

/* key-procedure */
void
BSnakeKey (BSnakeModule *pSnakeModule,
           BModuleKey    Key)
{
  switch (Key)
  {
    case B_KEY_2: /* set snake direction to up */
      if (pSnakeModule->SnakeLastDir.Y < 1)  /* not if last step was down */
      {
        pSnakeModule->SnakeDir.X = 0;
        pSnakeModule->SnakeDir.Y = -1;
      }
      break;
    case B_KEY_4: /* set snake direction to left */
      if (pSnakeModule->SnakeLastDir.X < 1)  /* not if last step was right */
      {
        pSnakeModule->SnakeDir.X = -1;
        pSnakeModule->SnakeDir.Y = 0;
      }
      break;
    case B_KEY_6: /* set snake direction to right */
      if (pSnakeModule->SnakeLastDir.X > -1)  /* not if last step was left */
      {
        pSnakeModule->SnakeDir.X = 1;
        pSnakeModule->SnakeDir.Y = 0;
      }
      break;
    case B_KEY_8: /* set snake direction to down */
      if (pSnakeModule->SnakeLastDir.Y > -1)  /* not if last step was up */
      {
        pSnakeModule->SnakeDir.X = 0;
        pSnakeModule->SnakeDir.Y = 1;
      }
      break;
    default:
      break;
  }
}

/* tick-procedure */
/* returns TRUE if game over or FALSE if game not over */
static gboolean
BSnakeTick (BSnakeModule *pSnakeModule)
{
  stBSnakePoint NewPos;
  gint I, Crash;

  /* snake is dead */
  if (pSnakeModule->DeadCnt > 0)
  {
    /* track time snake is dead */
    pSnakeModule->DeadCnt++;

    /* output current picture */
    BSnakeOutput (pSnakeModule);

    /* return game over when snake was dead long enough */
    return pSnakeModule->DeadCnt > BSnakeDeadTime;
  }

  /* get direction of snake head after next step */
  NewPos.X = pSnakeModule->pSnake[0].X + pSnakeModule->SnakeDir.X;
  NewPos.Y = pSnakeModule->pSnake[0].Y + pSnakeModule->SnakeDir.Y;

  /* no mouse available */
  if (pSnakeModule->Mouse.X < 0 || pSnakeModule->Mouse.Y < 0)
  {
    /* generate new mouse */
    pSnakeModule->Mouse.X = rand () % pSnakeModule->Size.X;
    pSnakeModule->Mouse.Y = rand () % pSnakeModule->Size.Y;

    /* check if new mouse position is valid */
    /* (i.e. not exactly at snake position and not directly next to snake) */
    if (BSnakePointNear (pSnakeModule->Mouse, NewPos))
    {
      /* remove mouse */
      pSnakeModule->Mouse.X = -1;
      pSnakeModule->Mouse.Y = -1;
    }
    for (I = 0; I < pSnakeModule->SnakeLen; I++)
    {
      if (BSnakePointNear (pSnakeModule->Mouse, pSnakeModule->pSnake[I]))
      {
        /* remove mouse */
        pSnakeModule->Mouse.X = -1;
        pSnakeModule->Mouse.Y = -1;
      }
    }

    if (pSnakeModule->Mouse.X >= 0 && pSnakeModule->Mouse.Y >= 0)
      dbg_print ("BSnake: new mouse\n");
  }

  /* snake eats mouse */
  if (pSnakeModule->Mouse.X >= 0
   && pSnakeModule->Mouse.Y >= 0
   && BSnakePointEqual (NewPos, pSnakeModule->Mouse))
  {
    /* remove mouse */
    pSnakeModule->Mouse.X = -1;
    pSnakeModule->Mouse.Y = -1;

    /* increase snake length */
    /* (last field of snake body will be set when moving snake one step) */
    if (pSnakeModule->SnakeLen < pSnakeModule->SnakeLenMax)
      pSnakeModule->SnakeLen++;

    /* increase game speed if snake already has got maximum length */
    /* (this formula will not increase the speed above maximum speed) */
    else
      pSnakeModule->Speed = (3 * pSnakeModule->Speed + BSnakeSpeedMax) / 4;

    dbg_print ("BSnake: snake ate mouse\n");
  }

  /* check if snake crashes */
  Crash = 0;
  if (NewPos.X < 0 /* snake crashes into wall */
   || NewPos.Y < 0
   || NewPos.X >= pSnakeModule->Size.X
   || NewPos.Y >= pSnakeModule->Size.Y)
    Crash = 1;
  for (I = 0; I < pSnakeModule->SnakeLen - 1; I++)  /* snake crashes into its body */
    if (BSnakePointEqual (NewPos, pSnakeModule->pSnake[I]))
      Crash = 1;

  /* snake crashes */
  if (Crash)
  {
    /* mark Snake as dead */
    pSnakeModule->DeadCnt = 1;

    dbg_print ("BSnake: snake crashed\n");
  }

  /* snake does not crash */
  else
  {
    /* move snake one step */
    for (I = pSnakeModule->SnakeLen; I > 0; I--)
      pSnakeModule->pSnake[I] = pSnakeModule->pSnake[I-1];
    pSnakeModule->pSnake[0] = NewPos;
    /* remember direction of last step */
    pSnakeModule->SnakeLastDir = pSnakeModule->SnakeDir;
  }

  /* output current picture */
  BSnakeOutput (pSnakeModule);

  /* return game not over */
  return FALSE;
}

static void
b_snake_module_class_init (BSnakeModuleClass *klass)
{
  BModuleClass *module_class;

  module_class = B_MODULE_CLASS (klass);

  module_class->query    = b_snake_module_query;
  module_class->prepare  = b_snake_module_prepare;
  module_class->relax    = b_snake_module_relax;
  module_class->start    = b_snake_module_start;
  module_class->event    = b_snake_module_event;
  module_class->tick     = b_snake_module_tick;
  module_class->describe = b_snake_module_describe;
}

static void
b_snake_module_init (BSnakeModule *snake_module)
{
}

static gboolean
b_snake_module_query (gint width,
                      gint height,
                      gint channels,
                      gint maxval)
{
  return (width >= BSnakeSizeXMin && height >= BSnakeSizeYMin
          && channels == 1 && maxval >= BSnakeMaxColorMin);
}

static gboolean
b_snake_module_prepare (BModule  *module,
                        GError  **error)
{
  BSnakeModule *snake = B_SNAKE_MODULE (module);

  /* initialize the module values that depend on the output device */
  snake->MaxColor = module->maxval; /* maximum color value */
  snake->Size.X = module->width; /* size of game field */
  snake->Size.Y = module->height;
  snake->SnakeLenIni = BSnakeLenIni (module->width, module->height); /* initial size of the snake */
  snake->SnakeLenMax = BSnakeLenMax (module->width, module->height); /* initial size of the snake */
  snake->PlayerId = -1; /* no player in game yet */

  snake->pSnake = g_new (stBSnakePoint, snake->SnakeLenMax);

  return TRUE;
}

static void
b_snake_module_relax (BModule  *module)
{
  BSnakeModule *snake = B_SNAKE_MODULE (module);

  g_free (snake->pSnake);
  snake->pSnake = NULL;
}

static void
b_snake_module_start (BModule *module)
{
  BSnakeModule *snake = B_SNAKE_MODULE (module);

  snake->Lives = BSnakeLives;
  BSnakeNewGame (snake);

  b_module_ticker_start (module, snake->Speed);
}

static void
b_snake_module_event (BModule      *module,
                      BModuleEvent *event)
{
  BSnakeModule *snake = B_SNAKE_MODULE (module);

  switch (event->type)
    {
    case B_EVENT_TYPE_KEY:
      if (snake->PlayerId == event->device_id)
        BSnakeKey (snake, event->key);
      break;

    case B_EVENT_TYPE_PLAYER_ENTERED:
     if (snake->PlayerId == -1)
       {
         snake->PlayerId = event->device_id;
         module->num_players++;
       }
      break;

    case B_EVENT_TYPE_PLAYER_LEFT:
      if (snake->PlayerId == event->device_id)
        {
          snake->PlayerId = -1;
          module->num_players--;
        }
      break;

    default:
      break;
    }
}

static gint
b_snake_module_tick (BModule *module)
{
  BSnakeModule *snake = B_SNAKE_MODULE (module);
  gboolean      GameOver;

  GameOver = BSnakeTick (snake);

  if (GameOver)
    {
      snake->Lives--;
      if (snake->Lives <= 0)
    {
      b_module_request_stop (module); /* request end */
      return 0;
    }
      BSnakeNewGame (snake);
  }

  /* we want to be called again in some milliseconds, if game is not over */
  return (snake->Speed);
}

static void
b_snake_module_describe (BModule      *module,
                         const gchar **title,
                         const gchar **description,
                         const gchar **author)
{
  *title       = "BSnake";
  *description = "Snake game "BSnakeVerTxt;
  *author      = "1stein <1stein@schuermans.info>";
}


/* Bxxo: tic-tac-toe module for Blinkenlights
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

#define BxxoVerMaj 1
#define BxxoVerMin 0
#define BxxoVerRev 0

/* minimum size and color count of display */
#define BxxoSizeXMin 12
#define BxxoSizeYMin 8
#define BxxoMaxColorMin 1
/* time player has to chose next field (in seconds) */
#define BxxoChoseTime 9
/* time to blink winning line (in seconds) */
#define BxxoWinTime 3
/* the colors */
#define BxxoColorEmpty(MaxColor)  (0)
#define BxxoColorField(MaxColor)  ((MaxColor) * 3 / 5)
#define BxxoColorMark(MaxColor)   (MaxColor)
#define BxxoColorNumber(MaxColor) (MaxColor)

#ifdef XXO_DEBUG
#define dbg_print g_print
#else
#define dbg_print(fmt, ...)
#endif

#define B_TYPE_XXO_MODULE         (b_type_xxo_module)
#define B_XXO_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_XXO_MODULE, BxxoModule))
#define B_XXO_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_XXO_MODULE, BxxoModuleClass))
#define B_IS_XXO_MODULE(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_XXO_MODULE))

typedef struct _BxxoModule BxxoModule;
typedef struct _BxxoModuleClass BxxoModuleClass;

struct _BxxoModule
{
  BModule parent_instance;

  gint MaxColor;                 /* maximum color value */
  gint SizeX, SizeY;             /* size of game-field */
  gint MarkX, MarkY;             /* size of a mark */
  gint PlayerAvail[2];           /* boolean flags if players are available */
  gint PlayerId[2];              /* id of player */
  gint Field[3][3];              /* game field: Field[Y][X], -1 = free, 0 = first player, 1 = second player */
  gint Player;                   /* number of player who has to chose a field next */
  gint ChoseTime;                /* time to chose (seconds) */
  gint Ticks;                    /* tick counter (0.1 seconds) */
  gint GameOver;                 /* boolean value set if game over */
  gint WinX, WinY, WinDX, WinDY; /* variables indicating winning line */
  gint WinTime;                  /* time to let winning line blink (seconds) */
};

struct _BxxoModuleClass
{
  BModuleClass parent_class;
};

static GType    b_xxo_module_get_type   (GTypeModule       *module);
static void     b_xxo_module_class_init (BxxoModuleClass   *klass);
static void     b_xxo_module_init       (BxxoModule        *test_module);
static gboolean b_xxo_module_query      (gint               width,
                                         gint               height,
                                         gint               channels,
                                         gint               maxval);
static gboolean b_xxo_module_prepare    (BModule           *module,
                                         GError           **error);
static void     b_xxo_module_relax      (BModule           *module);
static void     b_xxo_module_start      (BModule           *module);
static void     b_xxo_module_event      (BModule           *module,
                                         BModuleEvent      *event);
static gint     b_xxo_module_tick       (BModule           *module);
static void     b_xxo_module_describe   (BModule           *module,
                                         const gchar      **title,
                                         const gchar      **description,
                                         const gchar      **author);


static GType b_type_xxo_module = 0;

/* output a player mark */
static void BxxoOutMark (BxxoModule * pBxxoModule,
                         gint StartX, gint StartY,
                         gint SizeX, gint SizeY,
                         gint Player)
{
  gint X, Y, Con, Coff;
  Con = BxxoColorMark (pBxxoModule->MaxColor);
  Coff = BxxoColorEmpty (pBxxoModule->MaxColor);
  for (Y = 0; Y < SizeY; Y++)
    for (X = 0; X < SizeX; X++)
      if (Player == 0 || (Player == 1 && ((X + Y) & 1) == 0))
        b_module_draw_point ((BModule*)pBxxoModule, StartX + X, StartY + Y,
                             Con);
      else
        b_module_draw_point ((BModule*)pBxxoModule, StartX + X, StartY + Y,
                             Coff);
}


/* output a number (0..9) (3x5 pixels) */
const gchar BxxoNumbers[11][5][3] =
{
  {
    {1,1,1},
    {1,0,1},
    {1,0,1},
    {1,0,1},
    {1,1,1}
  },
  {
    {0,0,1},
    {0,0,1},
    {0,0,1},
    {0,0,1},
    {0,0,1}
  },
  {
    {1,1,1},
    {0,0,1},
    {1,1,1},
    {1,0,0},
    {1,1,1}
  },
  {
    {1,1,1},
    {0,0,1},
    {1,1,1},
    {0,0,1},
    {1,1,1}
  },
  {
    {1,0,1},
    {1,0,1},
    {1,1,1},
    {0,0,1},
    {0,0,1}
  },
  {
    {1,1,1},
    {1,0,0},
    {1,1,1},
    {0,0,1},
    {1,1,1}
  },
  {
    {1,1,1},
    {1,0,0},
    {1,1,1},
    {1,0,1},
    {1,1,1}
  },
  {
    {1,1,1},
    {0,0,1},
    {0,0,1},
    {0,0,1},
    {0,0,1}
  },
  {
    {1,1,1},
    {1,0,1},
    {1,1,1},
    {1,0,1},
    {1,1,1}
  },
  {
    {1,1,1},
    {1,0,1},
    {1,1,1},
    {0,0,1},
    {1,1,1}
  },
  {
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0},
    {0,0,0}
  }
};
static void BxxoOutNumber (BxxoModule * pBxxoModule,
                           gint X, gint Y,
                           gint N)
{
  gint Con, Coff, XX, YY;
  Con = BxxoColorNumber (pBxxoModule->MaxColor);
  Coff = BxxoColorEmpty (pBxxoModule->MaxColor);
  if (N > 9) N %= 10;
  if (N < 0) N = 10;
  for (YY = 0; YY < 5; YY++)
    for (XX = 0; XX < 3; XX++)
      b_module_draw_point ((BModule*)pBxxoModule, X + XX, Y + YY,
                           BxxoNumbers[N][YY][XX] ? Con : Coff);
}

/* check if current player has won */
static gint BxxoWinCheck (BxxoModule * pBxxoModule)
{
  gint X, Y, Cnt;
  /* horizontal */
  for (Y = 0; Y < 3; Y++)
  {
    for (X = 0; X < 3; X++)
      if (pBxxoModule->Field[Y][X] != pBxxoModule->Player)
        break;
    if (X >= 3)
    {
      pBxxoModule->WinX = 0;
      pBxxoModule->WinY = Y;
      pBxxoModule->WinDX = 1;
      pBxxoModule->WinDY = 0;
      return 1;
    }
  }
  /* vertical */
  for (X = 0; X < 3; X++)
  {
    for (Y = 0; Y < 3; Y++)
      if (pBxxoModule->Field[Y][X] != pBxxoModule->Player)
        break;
    if (Y >= 3)
    {
      pBxxoModule->WinX = X;
      pBxxoModule->WinY = 0;
      pBxxoModule->WinDX = 0;
      pBxxoModule->WinDY = 1;
      return 1;
    }
  }
  /* diagonal */
  for (Y = 0, X = 0; Y < 3; Y++, X++)
    if (pBxxoModule->Field[Y][X] != pBxxoModule->Player)
      break;
  if (Y >= 3)
  {
    pBxxoModule->WinX = 0;
    pBxxoModule->WinY = 0;
    pBxxoModule->WinDX = 1;
    pBxxoModule->WinDY = 1;
    return 1;
  }
  /* reverse diagonal */
  for (Y = 0, X = 2; Y < 3; Y++, X--)
    if (pBxxoModule->Field[Y][X] != pBxxoModule->Player)
      break;
  if (Y >= 3)
  {
    pBxxoModule->WinX = 2;
    pBxxoModule->WinY = 0;
    pBxxoModule->WinDX = -1;
    pBxxoModule->WinDY = 1;
    return 1;
  }
  /* no more free fields */
  Cnt = 0;
  for (Y = 0; Y < 3; Y++)
    for (X = 0; X < 3; X++)
      if (pBxxoModule->Field[Y][X] == -1)
        Cnt++;
  if (Cnt == 0)
  {
    pBxxoModule->WinX = -1;
    pBxxoModule->WinY = -1;
    pBxxoModule->WinDX = 0;
    pBxxoModule->WinDY = 0;
    return 1;
  }
  return 0;
}

/* chose a field for current player */
static void BxxoModuleChose (BxxoModule * pBxxoModule, gint * pX, gint * pY)
{
  gint X, Y, Cnt, I;

  /* try to win */

  /* horizontal */
  for (Y = 0; Y < 3; Y++)
  {
    if (pBxxoModule->Field[Y][0] == pBxxoModule->Player
     && pBxxoModule->Field[Y][1] == pBxxoModule->Player
     && pBxxoModule->Field[Y][2] == -1)
    {
      *pX = 2;
      *pY = Y;
      return;
    }
    if (pBxxoModule->Field[Y][0] == pBxxoModule->Player
     && pBxxoModule->Field[Y][1] == -1
     && pBxxoModule->Field[Y][2] == pBxxoModule->Player)
    {
      *pX = 1;
      *pY = Y;
      return;
    }
    if (pBxxoModule->Field[Y][0] == -1
     && pBxxoModule->Field[Y][1] == pBxxoModule->Player
     && pBxxoModule->Field[Y][2] == pBxxoModule->Player)
    {
      *pX = 0;
      *pY = Y;
      return;
    }
  }
  /* vertical */
  for (X = 0; X < 3; X++)
  {
    if (pBxxoModule->Field[0][X] == pBxxoModule->Player
     && pBxxoModule->Field[1][X] == pBxxoModule->Player
     && pBxxoModule->Field[2][X] == -1)
    {
      *pX = X;
      *pY = 2;
      return;
    }
    if (pBxxoModule->Field[0][X] == pBxxoModule->Player
     && pBxxoModule->Field[1][X] == -1
     && pBxxoModule->Field[2][X] == pBxxoModule->Player)
    {
      *pX = X;
      *pY = 1;
      return;
    }
    if (pBxxoModule->Field[0][X] == -1
     && pBxxoModule->Field[1][X] == pBxxoModule->Player
     && pBxxoModule->Field[2][X] == pBxxoModule->Player)
    {
      *pX = X;
      *pY = 0;
      return;
    }
  }
  /* diagonal */
  if (pBxxoModule->Field[0][0] == pBxxoModule->Player
   && pBxxoModule->Field[1][1] == pBxxoModule->Player
   && pBxxoModule->Field[2][2] == -1)
  {
    *pX = 2;
    *pY = 2;
    return;
  }
  if (pBxxoModule->Field[0][0] == pBxxoModule->Player
   && pBxxoModule->Field[1][1] == -1
   && pBxxoModule->Field[2][2] == pBxxoModule->Player)
  {
    *pX = 1;
    *pY = 1;
    return;
  }
  if (pBxxoModule->Field[0][0] == -1
   && pBxxoModule->Field[1][1] == pBxxoModule->Player
   && pBxxoModule->Field[2][2] == pBxxoModule->Player)
  {
    *pX = 0;
    *pY = 0;
    return;
  }
  /* reverse diagonal */
  if (pBxxoModule->Field[0][2] == pBxxoModule->Player
   && pBxxoModule->Field[1][1] == pBxxoModule->Player
   && pBxxoModule->Field[2][0] == -1)
  {
    *pX = 0;
    *pY = 2;
    return;
  }
  if (pBxxoModule->Field[0][2] == pBxxoModule->Player
   && pBxxoModule->Field[1][1] == -1
   && pBxxoModule->Field[2][0] == pBxxoModule->Player)
  {
    *pX = 1;
    *pY = 1;
    return;
  }
  if (pBxxoModule->Field[0][2] == -1
   && pBxxoModule->Field[1][1] == pBxxoModule->Player
   && pBxxoModule->Field[2][0] == pBxxoModule->Player)
  {
    *pX = 2;
    *pY = 0;
    return;
  }

  /* no direct chance to win */

  /* try to block other player */

  /* horizontal */
  for (Y = 0; Y < 3; Y++)
  {
    if (pBxxoModule->Field[Y][0] == 1 - pBxxoModule->Player
     && pBxxoModule->Field[Y][1] == 1 - pBxxoModule->Player
     && pBxxoModule->Field[Y][2] == -1)
    {
      *pX = 2;
      *pY = Y;
      return;
    }
    if (pBxxoModule->Field[Y][0] == 1 - pBxxoModule->Player
     && pBxxoModule->Field[Y][1] == -1
     && pBxxoModule->Field[Y][2] == 1 - pBxxoModule->Player)
    {
      *pX = 1;
      *pY = Y;
      return;
    }
    if (pBxxoModule->Field[Y][0] == -1
     && pBxxoModule->Field[Y][1] == 1 - pBxxoModule->Player
     && pBxxoModule->Field[Y][2] == 1 - pBxxoModule->Player)
    {
      *pX = 0;
      *pY = Y;
      return;
    }
  }
  /* vertical */
  for (X = 0; X < 3; X++)
  {
    if (pBxxoModule->Field[0][X] == 1 - pBxxoModule->Player
     && pBxxoModule->Field[1][X] == 1 - pBxxoModule->Player
     && pBxxoModule->Field[2][X] == -1)
    {
      *pX = X;
      *pY = 2;
      return;
    }
    if (pBxxoModule->Field[0][X] == 1 - pBxxoModule->Player
     && pBxxoModule->Field[1][X] == -1
     && pBxxoModule->Field[2][X] == 1 - pBxxoModule->Player)
    {
      *pX = X;
      *pY = 1;
      return;
    }
    if (pBxxoModule->Field[0][X] == -1
     && pBxxoModule->Field[1][X] == 1 - pBxxoModule->Player
     && pBxxoModule->Field[2][X] == 1 - pBxxoModule->Player)
    {
      *pX = X;
      *pY = 0;
      return;
    }
  }
  /* diagonal */
  if (pBxxoModule->Field[0][0] == 1 - pBxxoModule->Player
   && pBxxoModule->Field[1][1] == 1 - pBxxoModule->Player
   && pBxxoModule->Field[2][2] == -1)
  {
    *pX = 2;
    *pY = 2;
    return;
  }
  if (pBxxoModule->Field[0][0] == 1 - pBxxoModule->Player
   && pBxxoModule->Field[1][1] == -1
   && pBxxoModule->Field[2][2] == 1 - pBxxoModule->Player)
  {
    *pX = 1;
    *pY = 1;
    return;
  }
  if (pBxxoModule->Field[0][0] == -1
   && pBxxoModule->Field[1][1] == 1 - pBxxoModule->Player
   && pBxxoModule->Field[2][2] == 1 - pBxxoModule->Player)
  {
    *pX = 0;
    *pY = 0;
    return;
  }
  /* reverse diagonal */
  if (pBxxoModule->Field[0][2] == 1 - pBxxoModule->Player
   && pBxxoModule->Field[1][1] == 1 - pBxxoModule->Player
   && pBxxoModule->Field[2][0] == -1)
  {
    *pX = 0;
    *pY = 2;
    return;
  }
  if (pBxxoModule->Field[0][2] == 1 - pBxxoModule->Player
   && pBxxoModule->Field[1][1] == -1
   && pBxxoModule->Field[2][0] == 1 - pBxxoModule->Player)
  {
    *pX = 1;
    *pY = 1;
    return;
  }
  if (pBxxoModule->Field[0][2] == -1
   && pBxxoModule->Field[1][1] == 1 - pBxxoModule->Player
   && pBxxoModule->Field[2][0] == 1 - pBxxoModule->Player)
  {
    *pX = 2;
    *pY = 0;
    return;
  }

  /* no need to block other player */

  /* count free fields */
  Cnt = 0;
  for (Y = 0; Y < 3; Y++)
    for (X = 0; X < 3; X++)
      if (pBxxoModule->Field[Y][X] == -1)
        Cnt++;
  /* there is a free field */
  if (Cnt > 0)
  {
    /* select a random one */
    I = rand () % Cnt;
    /* return the I-th free field */
    for (Y = 0; Y < 3; Y++)
      for (X = 0; X < 3; X++)
        if (pBxxoModule->Field[Y][X] == -1)
        {
          if (I == 0)
          {
            *pX = X;
            *pY = Y;
            return;
          }
          I--;
        }
  }

  /* reaching this line should not occur */

  /* choose the middle field to be on the safe side and return indices within the field */
  *pX = 1;
  *pY = 1;
}

G_MODULE_EXPORT gboolean
b_module_register (GTypeModule * module)
{
  b_xxo_module_get_type (module);

  return TRUE;
}

GType
b_xxo_module_get_type (GTypeModule * module)
{
  if (!b_type_xxo_module)
    {
      static const GTypeInfo xxo_module_info = {
	sizeof (BxxoModuleClass),
	NULL,			/* base_init */
	NULL,			/* base_finalize */
	(GClassInitFunc) b_xxo_module_class_init,
	NULL,			/* class_finalize */
	NULL,			/* class_data */
	sizeof (BxxoModule),
	0,			/* n_preallocs */
	(GInstanceInitFunc) b_xxo_module_init,
      };

      /* !!!!!!!!! The name given in the next function MUST be unique! */

      b_type_xxo_module = g_type_module_register_type (module,
						       B_TYPE_MODULE,
						       "Bxxo",
						       &xxo_module_info,
						       0);
    }

  return b_type_xxo_module;
}

static void
b_xxo_module_class_init (BxxoModuleClass *klass)
{
  BModuleClass *module_class;

  module_class = B_MODULE_CLASS (klass);

  module_class->query    = b_xxo_module_query;
  module_class->prepare  = b_xxo_module_prepare;
  module_class->relax    = b_xxo_module_relax;
  module_class->start    = b_xxo_module_start;
  module_class->event    = b_xxo_module_event;
  module_class->tick     = b_xxo_module_tick;
  module_class->describe = b_xxo_module_describe;
}

static void
b_xxo_module_init (BxxoModule *pBxxoModule)
{
}

static gboolean
b_xxo_module_query (gint width,
                      gint height,
                      gint channels,
                      gint maxval)
{
  return (width >= BxxoSizeXMin && height >= BxxoSizeYMin
	  && channels == 1 && maxval >= BxxoMaxColorMin);
}

static gboolean
b_xxo_module_prepare (BModule  *module,
                        GError  **error)
{
  BxxoModule *pBxxoModule = B_XXO_MODULE (module);

  /* initialize the module values that depend on the output device */
  pBxxoModule->MaxColor = module->maxval; /* maximum color value */
  pBxxoModule->SizeX    = module->width; /* size of game field */
  pBxxoModule->SizeY    = module->height;
  pBxxoModule->MarkX    = (pBxxoModule->SizeX - 6) / 3; /* size of a mark */
  pBxxoModule->MarkY    = (pBxxoModule->SizeY - 2) / 3;

  return TRUE;
}

static void
b_xxo_module_relax (BModule *module)
{
}

static void
b_xxo_module_start (BModule *module)
{
  BxxoModule *pBxxoModule = B_XXO_MODULE (module);
  gint X, Y;

  /* no player available yet */
  pBxxoModule->PlayerAvail[0] = 0;
  pBxxoModule->PlayerAvail[1] = 0;
  /* game field is empty */
  for (Y = 0; Y < 3; Y++)
    for (X = 0; X < 3; X++)
      pBxxoModule->Field[Y][X] = -1;
  /* player 1 starts */
  pBxxoModule->Player = 0;
  pBxxoModule->ChoseTime = BxxoChoseTime;
  /* no ticks in current second yet */
  pBxxoModule->Ticks = 0;
  /* game not over */
  pBxxoModule->GameOver = 0;

  /* initialize screen */
  b_module_fill ((BModule *) pBxxoModule, /* empty the screen */
                 BxxoColorEmpty (pBxxoModule->MaxColor));
  b_module_draw_line ((BModule *) pBxxoModule, /* draw the field */
                      pBxxoModule->MarkX, 0,
                      pBxxoModule->MarkX, pBxxoModule->SizeY - 1,
                      BxxoColorField (pBxxoModule->MaxColor));
  b_module_draw_line ((BModule *) pBxxoModule,
                      pBxxoModule->MarkX * 2 + 1, 0,
                      pBxxoModule->MarkX * 2 + 1, pBxxoModule->SizeY - 1,
                      BxxoColorField (pBxxoModule->MaxColor));
  b_module_draw_line ((BModule *) pBxxoModule,
                      0, pBxxoModule->MarkY,
                      pBxxoModule->SizeX - 5, pBxxoModule->MarkY,
                      BxxoColorField (pBxxoModule->MaxColor));
  b_module_draw_line ((BModule *) pBxxoModule,
                      0, pBxxoModule->MarkY * 2 + 1,
                      pBxxoModule->SizeX - 5, pBxxoModule->MarkY * 2 + 1,
                      BxxoColorField (pBxxoModule->MaxColor));
  BxxoOutNumber (pBxxoModule, pBxxoModule->SizeX - 3, pBxxoModule->SizeY - 5,
                 pBxxoModule->ChoseTime);
  b_module_paint ((BModule *) pBxxoModule); /* put image onto screen */

  /* start the tick machinery */
  b_module_ticker_start (module, 100);
}

static void
b_xxo_module_event (BModule      *module,
                    BModuleEvent *event)
{
  gint I, X, Y;

  BxxoModule *pBxxoModule = B_XXO_MODULE (module);

  /* ignore events if game over */
  if (pBxxoModule->GameOver)
    return;

  /* player entered */
  if (event->type == B_EVENT_TYPE_PLAYER_ENTERED)
  {
    /* search for player not available */
    for (I = 0; I < 2; I++)
      if (!pBxxoModule->PlayerAvail[I])
        break;
    /* found player */
    if (I < 2)
    {
      /* player joins */
      pBxxoModule->PlayerAvail[I] = 1;
      pBxxoModule->PlayerId[I] = event->device_id;
      dbg_print ("Bxxo: player %d (dev_id %d) joined\n", I, event->device_id);
      /* show player indicator */
      if (I == pBxxoModule->Player)
      {
        BxxoOutMark (pBxxoModule, pBxxoModule->SizeX - 3, 0,
                     3, pBxxoModule->MarkY, I);
        b_module_paint ((BModule *) pBxxoModule); /* put image onto screen */
      }
    }
  }

  /* player left */
  else if (event->type == B_EVENT_TYPE_PLAYER_LEFT)
  {
    /* search for player with device-id */
    for (I = 0; I < 2; I++)
      if (pBxxoModule->PlayerAvail[I]
          && pBxxoModule->PlayerId[I] == event->device_id)
        break;
    /* found player */
    if (I < 2)
    {
      /* player leaves */
      pBxxoModule->PlayerAvail[I] = 0;
      dbg_print ("Bxxo: player %d (dev_id %d) left\n", I, event->device_id);
      /* remove player indicator */
      if (I == pBxxoModule->Player)
      {
        BxxoOutMark (pBxxoModule, pBxxoModule->SizeX - 3, 0,
                     3, pBxxoModule->MarkY, -1);
        b_module_paint ((BModule *) pBxxoModule); /* put image onto screen */
      }
    }
  }

  /* key-event */
  else if (event->type == B_EVENT_TYPE_KEY)
  {
    /* search for player with device-id */
    for (I = 0; I < 2; I++)
      if (pBxxoModule->PlayerAvail[I]
          && pBxxoModule->PlayerId[I] == event->device_id)
        break;
    /* found player */
    if (I < 2)
    {
      /* get position from key */
      switch (event->key)
      {
        case B_KEY_1: X = 0; Y = 0; break;
        case B_KEY_2: X = 1; Y = 0; break;
        case B_KEY_3: X = 2; Y = 0; break;
        case B_KEY_4: X = 0; Y = 1; break;
        case B_KEY_5: X = 1; Y = 1; break;
        case B_KEY_6: X = 2; Y = 1; break;
        case B_KEY_7: X = 0; Y = 2; break;
        case B_KEY_8: X = 1; Y = 2; break;
        case B_KEY_9: X = 2; Y = 2; break;
        default: X = -1; Y = -1;
      }
      /* a valid key was pressed by the active player */
      if (X >= 0 && Y >= 0 && I == pBxxoModule->Player)
      {
        /* check if position is still free */
        if (pBxxoModule->Field[Y][X] < 0)
        {
          dbg_print ("Bxxo: player %d (dev_id %d) chose field (%d,%d)\n",
                     I, event->device_id, X, Y);
          /* place mark */
          pBxxoModule->Field[Y][X] = I;
          BxxoOutMark (pBxxoModule,
                       (pBxxoModule->MarkX + 1) * X,
                       (pBxxoModule->MarkY + 1) * Y,
                       pBxxoModule->MarkX, pBxxoModule->MarkY, I);
          /* player has won */
          if (BxxoWinCheck (pBxxoModule))
          {
            /* game is over */
            pBxxoModule->GameOver = 1;
            /* start win time */
            pBxxoModule->ChoseTime = -1;
            pBxxoModule->WinTime = BxxoWinTime;
            /* no ticks in current second yet */
            pBxxoModule->Ticks = 0;
          }
          /* player has not won */
          else
          {
            /* switch to other player */
            pBxxoModule->Player = 1 - pBxxoModule->Player;
            pBxxoModule->ChoseTime = BxxoChoseTime;
            /* no ticks in current second yet */
            pBxxoModule->Ticks = 0;
          }

          /* update player indicator */
          if (pBxxoModule->PlayerAvail[pBxxoModule->Player])
            BxxoOutMark (pBxxoModule, pBxxoModule->SizeX - 3, 0,
                         3, pBxxoModule->MarkY, pBxxoModule->Player);
          else
            BxxoOutMark (pBxxoModule, pBxxoModule->SizeX - 3, 0,
                         3, pBxxoModule->MarkY, -1);
          /* show choose time */
          BxxoOutNumber (pBxxoModule, pBxxoModule->SizeX - 3,
                         pBxxoModule->SizeY - 5,
                         pBxxoModule->ChoseTime);
          /* put image onto screen */
          b_module_paint ((BModule *) pBxxoModule);
        }
      }
    }
  }
}

static gint
b_xxo_module_tick (BModule *module)
{
  BxxoModule *pBxxoModule = B_XXO_MODULE (module);
  gint X, Y, I;

  /* game over - blink with winning line */
  if (pBxxoModule->GameOver && pBxxoModule->WinX >= 0 && pBxxoModule->WinY >= 0)
  {
    /* off */
    if (pBxxoModule->Ticks == 0)
    {
      for (I = 0, X = pBxxoModule->WinX, Y = pBxxoModule->WinY;
           I < 3; I++, X += pBxxoModule->WinDX, Y += pBxxoModule->WinDY)
        BxxoOutMark (pBxxoModule,
                     (pBxxoModule->MarkX + 1) * X, (pBxxoModule->MarkY + 1) * Y,
                     pBxxoModule->MarkX, pBxxoModule->MarkY,
                     -1);
      b_module_paint ((BModule *) pBxxoModule);
    }
    /* on */
    if (pBxxoModule->Ticks == 5)
    {
      for (I = 0, X = pBxxoModule->WinX, Y = pBxxoModule->WinY;
           I < 3; I++, X += pBxxoModule->WinDX, Y += pBxxoModule->WinDY)
        BxxoOutMark (pBxxoModule,
                     (pBxxoModule->MarkX + 1) * X, (pBxxoModule->MarkY + 1) * Y,
                     pBxxoModule->MarkX, pBxxoModule->MarkY,
                     pBxxoModule->Player);
      b_module_paint ((BModule *) pBxxoModule);
    }
  } /* if (pBxxModule->GameOver ... */

  /* count ticks */
  pBxxoModule->Ticks++;

  /* one second elapsed */
  if (pBxxoModule->Ticks >= 10)
  {
    pBxxoModule->Ticks = 0;

    /* game over */
    if (pBxxoModule->GameOver)
    {
      /* decrease win time */
      pBxxoModule->WinTime--;

      /* win time reached zero */
      if (pBxxoModule->WinTime <= 0)
      {
        dbg_print ("Bxxo: requesting stop\n");
        /* clear screen */
        b_module_fill (module, 0);
        b_module_paint (module);
        /* request end of game */
        b_module_request_stop (module);
        /* we do not want to be called again */
        return 0;
      }
    } /* game over */

    /* game not over */
    else
    {
      /* decrease choose time */
      pBxxoModule->ChoseTime--;

      /* choose time reached zero */
      if (pBxxoModule->ChoseTime <= 0)
      {
        /* find position to place mark */
        BxxoModuleChose (pBxxoModule, &X, &Y);
        dbg_print ("Bxxo: computer chose field (%d,%d) for player %d\n",
                   X, Y, pBxxoModule->Player);
        /* place mark */
        pBxxoModule->Field[Y][X] = pBxxoModule->Player;
        BxxoOutMark (pBxxoModule,
                     (pBxxoModule->MarkX + 1) * X, (pBxxoModule->MarkY + 1) * Y,
                     pBxxoModule->MarkX, pBxxoModule->MarkY, pBxxoModule->Player);
        /* player has won */
        if (BxxoWinCheck (pBxxoModule))
        {
          /* game is over */
          pBxxoModule->GameOver = 1;
          /* start win time */
          pBxxoModule->ChoseTime = -1;
          pBxxoModule->WinTime = BxxoWinTime;
          /* no ticks in current second yet */
          pBxxoModule->Ticks = 0;
        }
        /* player has not won */
        else
        {
          /* switch to other player */
          pBxxoModule->Player = 1 - pBxxoModule->Player;
          pBxxoModule->ChoseTime = BxxoChoseTime;
          /* no ticks in current second yet */
          pBxxoModule->Ticks = 0;
        }

        /* update player indicator */
        if (pBxxoModule->PlayerAvail[pBxxoModule->Player])
          BxxoOutMark (pBxxoModule, pBxxoModule->SizeX - 3, 0,
                       3, pBxxoModule->MarkY, pBxxoModule->Player);
        else
          BxxoOutMark (pBxxoModule, pBxxoModule->SizeX - 3, 0,
                       3, pBxxoModule->MarkY, -1);
      }

      /* show choose time */
      BxxoOutNumber (pBxxoModule, pBxxoModule->SizeX - 3,
                     pBxxoModule->SizeY - 5,
                     pBxxoModule->ChoseTime);
      /* put image onto screen */
      b_module_paint ((BModule *) pBxxoModule);
    } /* game not over */
  } /* one second elapsed */

  /* we want to be called again in 100 milliseconds */
  return 100;
}

static void
b_xxo_module_describe (BModule      *module,
                       const gchar **title,
                       const gchar **description,
                       const gchar **author)
{
  *title       = "Bxxo";
  *description = "tic-tac-toe game";
  *author      = "1stein";
}


/* BBreakout: Breakout module for Blinkenlights
 *
 * Copyright (c) 2002  1stein <1stein@1stein.no-ip.com>
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

#define BBreakoutVerMaj 1
#define BBreakoutVerMin 0
#define BBreakoutVerRev 0

/* minimum size and color count of display */
#define BBreakoutSizeXMin 18
#define BBreakoutSizeYMin 15
#define BBreakoutColorCntMin 4
/* minimum number of free pixel rows over player */
#define BBreakoutFreeRowsMin 3
/* size of stones */
#define BBreakoutStoneSizeX 3
#define BBreakoutStoneSizeY 1
/* number of lives for player */
#define BBreakoutLives 5
/* speed of game */
#define BBreakoutTicks 120
/* number of ticks player is dead after missing a ball */
#define BBreakoutDeadTime 10

#define BBreakoutAutoStart TRUE

/* the colors */
#define BBreakoutColorEmpty(MaxColor)           (0)
#define BBreakoutColorStone1(MaxColor)          ((MaxColor) / 2)
#define BBreakoutColorStone2(MaxColor)          ((MaxColor) * 3 / 4)
#define BBreakoutColorStone3(MaxColor)          (MaxColor)
#define BBreakoutColorBall(MaxColor)            (MaxColor)
#define BBreakoutColorPlayerDead(MaxColor)      ((MaxColor) / 2)
#define BBreakoutColorPlayerDeadBlink(MaxColor) ((MaxColor) / 4)
#define BBreakoutColorPlayerAlive(MaxColor)     ((MaxColor) * 3 / 4)

#ifdef BREAKOUT_DEBUG
#define dbg_print g_print
#else
static inline void
dbg_print (gchar * Fmt, ...)
{
}
#endif

#define B_TYPE_BREAKOUT_MODULE         (b_type_breakout_module)
#define B_BREAKOUT_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_BREAKOUT_MODULE, BBreakoutModule))
#define B_BREAKOUT_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_BREAKOUT_MODULE, BBreakoutModuleClass))
#define B_IS_BREAKOUT_MODULE(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_BREAKOUT_MODULE))

typedef struct _BBreakoutModule      BBreakoutModule;
typedef struct _BBreakoutModuleClass BBreakoutModuleClass;

struct _BBreakoutModule
{
  BModule parent_instance;

  gint    MaxColor;                   /* maximum color value */
  gint    FreeRows;                   /* number of free pixel rows over player */
  gint    StoneCntX, StoneCntY;       /* number of stones in field */
  gint    StoneSizeX, StoneSizeY;     /* size of stone area of game-field */
  gint    SizeX, SizeY;               /* size of game-field */
  gint    OfsX, OfsY;                 /* offset of top left corner of game-field */
  gint    PlayerSizeX, PlayerPosY;    /* x-size and y-position of player */
  gint    PlayerPosXMax, PlayerStepX; /* maximum x-position and x-step-size of player */
  gint  **ppStones;                   /* two-dimensional array with stones */
  gint    StoneCnt;                   /* number of stones left in game */
  gint    Lives;                      /* rest of player's lives */
  gint    DeadCnt;                    /* number of ticks player is dead, 0 if player is alive */
  gint    PlayerPosX;                 /* x-position of (left corner of) player */
  gint    BallActive;                 /* !0 if ball is flying, 0 if ball is at player */
  gint    BallPosX, BallPosY;         /* position of ball (if ball is flying) */
  gint    BallDirX, BallDirY;         /* direction of ball (if ball is flying) */
 
  gint    player_device_id;
};

struct _BBreakoutModuleClass
{
  BModuleClass parent_class;
};
static GType    b_breakout_module_get_type   (GTypeModule           *module);
static void     b_breakout_module_class_init (BBreakoutModuleClass  *klass);
static void     b_breakout_module_init       (BBreakoutModule       *module);
static gboolean b_breakout_module_query      (gint                   width,
                                              gint                   height,
                                              gint                   channels,
                                              gint                   maxval);
static gboolean b_breakout_module_prepare    (BModule               *module,
                                              GError               **error);
static void     b_breakout_module_relax      (BModule               *module);
static void     b_breakout_module_start      (BModule               *module);
static void     b_breakout_module_event      (BModule               *module,
                                              BModuleEvent          *event);
static gint     b_breakout_module_tick       (BModule               *module);
static void     b_breakout_module_describe   (BModule               *module,
                                              const gchar          **title,
                                              const gchar          **description,
                                              const gchar          **author);



static GType b_type_breakout_module = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule * module)
{
  b_breakout_module_get_type (module);

  return TRUE;
}

GType
b_breakout_module_get_type (GTypeModule * module)
{
  if (!b_type_breakout_module)
    {
      static const GTypeInfo breakout_module_info = {
	sizeof (BBreakoutModuleClass),
	NULL,			/* base_init */
	NULL,			/* base_finalize */
	(GClassInitFunc) b_breakout_module_class_init,
	NULL,			/* class_finalize */
	NULL,			/* class_data */
	sizeof (BBreakoutModule),
	0,			/* n_preallocs */
	(GInstanceInitFunc) b_breakout_module_init,
      };

      /* !!!!!!!!! The name given in the next function MUST be unique! */

      b_type_breakout_module = g_type_module_register_type (module,
							    B_TYPE_MODULE,
							    "BBreakout",
							    &breakout_module_info,
							    0);
    }

  return b_type_breakout_module;
}

/* output current picture */
void
BBreakoutOutput (BBreakoutModule *pBreakoutModule)
{
  gint OfsX, OfsY, X, Y, X1, Y1, Y2, Color;

  /* empty the screen */
  b_module_fill ((BModule *) pBreakoutModule,
		 BBreakoutColorEmpty (pBreakoutModule->MaxColor));

  /* get offset */
  OfsX = pBreakoutModule->OfsX;
  OfsY = pBreakoutModule->OfsY;

  /* draw stones */
  for (Y = 0; Y < pBreakoutModule->StoneCntY; Y++)
  {
    for (X = 0; X < pBreakoutModule->StoneCntX; X++)
    {
      /* stone there */
      if (pBreakoutModule->ppStones[Y][X] > 0)
      {
        /* get position of stone */
        X1 = OfsX + X * BBreakoutStoneSizeX;
        Y1 = OfsY + Y * BBreakoutStoneSizeY;
        /* get color */
        if (pBreakoutModule->ppStones[Y][X] == 1)
          Color = BBreakoutColorStone1 (pBreakoutModule->MaxColor);
        else if (pBreakoutModule->ppStones[Y][X] == 2)
          Color = BBreakoutColorStone2 (pBreakoutModule->MaxColor);
        else
          Color = BBreakoutColorStone3 (pBreakoutModule->MaxColor);
        /* draw stone */
        for (Y2 = 0; Y2 < BBreakoutStoneSizeY; Y2++)
          b_module_draw_line ((BModule *) pBreakoutModule,
                              X1, Y1 + Y2,
                              X1 + BBreakoutStoneSizeX - 1, Y1 + Y2,
                              Color);
      }
    }
  }

  /* get player color (dead or alive) */
  if (pBreakoutModule->DeadCnt > 0)
  {
    if (pBreakoutModule->DeadCnt & 1)
      Color = BBreakoutColorPlayerDead (pBreakoutModule->MaxColor);
    else
      Color = BBreakoutColorPlayerDeadBlink (pBreakoutModule->MaxColor);
  }
  else
    Color = BBreakoutColorPlayerAlive (pBreakoutModule->MaxColor);
  /* draw player */
  b_module_draw_line ((BModule *) pBreakoutModule,
                      OfsX + pBreakoutModule->PlayerPosX,
                      OfsY + pBreakoutModule->PlayerPosY,
                      OfsX + pBreakoutModule->PlayerPosX
                           + pBreakoutModule->PlayerSizeX - 1,
                      OfsY + pBreakoutModule->PlayerPosY,
                      Color);

  /* draw active ball */
  if (pBreakoutModule->BallActive)
  {
    b_module_draw_point ((BModule *) pBreakoutModule,
                         OfsX + pBreakoutModule->BallPosX,
			 OfsY + pBreakoutModule->BallPosY,
			 BBreakoutColorBall (pBreakoutModule->MaxColor));
  }
  /* draw ball at player */
  else
  {
    b_module_draw_point ((BModule *) pBreakoutModule,
                         OfsX + pBreakoutModule->PlayerPosX
                              + pBreakoutModule->PlayerSizeX / 2,
			 OfsY + pBreakoutModule->PlayerPosY - 1,
			 BBreakoutColorBall (pBreakoutModule->MaxColor));
  }

  /* update screen */
  b_module_paint ((BModule *) pBreakoutModule);
}

/* initialize player */
void
BBreakoutPlayerInit (BBreakoutModule *pBreakoutModule)
{
  /* player is alive */
  pBreakoutModule->DeadCnt = 0;
  /* set start position of player */
  pBreakoutModule->PlayerPosX =
    (pBreakoutModule->SizeX - pBreakoutModule->PlayerSizeX) / 2;

  if (BBreakoutAutoStart)
    {
      /* set ball to active, set position and direction */
      pBreakoutModule->BallActive = 1;
      pBreakoutModule->BallPosX = pBreakoutModule->PlayerPosX
                                + pBreakoutModule->PlayerSizeX / 2;
      pBreakoutModule->BallPosY = pBreakoutModule->PlayerPosY - 1;
      pBreakoutModule->BallDirX = rand() % 1 ? -1 : 1;
      pBreakoutModule->BallDirY = -1;
    }
  else
    {
      /* ball is at player */
      pBreakoutModule->BallActive = 0;
    }
}

/* start a new game */
void
BBreakoutNewGame (BBreakoutModule *pBreakoutModule)
{
  gint X, Y;

  /* set stones to 2 or 3 hits in checkerboard-style */
  for (Y = 0; Y < pBreakoutModule->StoneCntY; Y++)
    for (X = 0; X < pBreakoutModule->StoneCntX; X++)
      if ((X + Y) & 1)
        pBreakoutModule->ppStones[Y][X] = 2;
      else
        pBreakoutModule->ppStones[Y][X] = 3;
  /* make a free way around the field */
  for (Y = 2; Y < pBreakoutModule->StoneCntY - 1; Y++)
  {
    pBreakoutModule->ppStones[Y][0] = 0;
    pBreakoutModule->ppStones[Y][pBreakoutModule->StoneCntX - 1] = 0;
  }
  for (X = 0; X < pBreakoutModule->StoneCntX; X++)
  {
    pBreakoutModule->ppStones[0][X] = 0;
    pBreakoutModule->ppStones[1][X] = 0;
  }
  /* put a hole into the upper center */
  for (Y = 4; Y < pBreakoutModule->StoneCntY * 2 / 3; Y++)
  {
    pBreakoutModule->ppStones[Y][(pBreakoutModule->StoneCntX - 1) / 2] = 0;
    pBreakoutModule->ppStones[Y][(pBreakoutModule->StoneCntX) / 2] = 0;
  }

  /* count stones */
  pBreakoutModule->StoneCnt = 0;
  for (Y = 0; Y < pBreakoutModule->StoneCntY; Y++)
    for (X = 0; X < pBreakoutModule->StoneCntX; X++)
      if (pBreakoutModule->ppStones[Y][X] > 0)
        pBreakoutModule->StoneCnt++;

  dbg_print ("BBreakout: new game\n");

  /* initialize player */
  BBreakoutPlayerInit (pBreakoutModule);

  /* output current picture */
  BBreakoutOutput (pBreakoutModule);
}

/* key-procedure */
void
BBreakoutKey (BBreakoutModule *pBreakoutModule,
              BModuleKey     Key)
{
  /* don't do anything if player is dead */
  if (pBreakoutModule->DeadCnt > 0)
	  return;

  switch (Key)
    {
    case B_KEY_1:
      /* shoot ball to the left */
      if (!pBreakoutModule->BallActive)
      {
        /* set ball to active, set position and direction */
        pBreakoutModule->BallActive = 1;
        pBreakoutModule->BallPosX   = pBreakoutModule->PlayerPosX
                                    + pBreakoutModule->PlayerSizeX / 2;
        pBreakoutModule->BallPosY   = pBreakoutModule->PlayerPosY - 1;
        pBreakoutModule->BallDirX   = -1;
        pBreakoutModule->BallDirY   = -1;
      }
      break;
    case B_KEY_3:
      /* shoot ball to the right */
      if (!pBreakoutModule->BallActive)
      {
        /* set ball to active, set position and direction */
        pBreakoutModule->BallActive = 1;
        pBreakoutModule->BallPosX   = pBreakoutModule->PlayerPosX
                                    + pBreakoutModule->PlayerSizeX / 2;
        pBreakoutModule->BallPosY   = pBreakoutModule->PlayerPosY - 1;
        pBreakoutModule->BallDirX   = 1;
        pBreakoutModule->BallDirY   = -1;
      }
      break;
    case B_KEY_4:
      /* move player one step left */
      pBreakoutModule->PlayerPosX -= pBreakoutModule->PlayerStepX;
      if (pBreakoutModule->PlayerPosX < 0)
        pBreakoutModule->PlayerPosX = 0;
      break;
    case B_KEY_6:
      /* move player one step right */
      pBreakoutModule->PlayerPosX += pBreakoutModule->PlayerStepX;
      if (pBreakoutModule->PlayerPosX > pBreakoutModule->PlayerPosXMax)
        pBreakoutModule->PlayerPosX = pBreakoutModule->PlayerPosXMax;
      break;
    case B_KEY_7:
      /* move player one pixel left */
      pBreakoutModule->PlayerPosX--;;
      if (pBreakoutModule->PlayerPosX < 0)
        pBreakoutModule->PlayerPosX = 0;
      break;
    case B_KEY_9:
      /* move player one pixel right */
      pBreakoutModule->PlayerPosX++;
      if (pBreakoutModule->PlayerPosX > pBreakoutModule->PlayerPosXMax)
        pBreakoutModule->PlayerPosX = pBreakoutModule->PlayerPosXMax;
      break;

    default:
      break;
    }

  /* output current picture */
  BBreakoutOutput (pBreakoutModule);
}

/* tick-procedure */
/* returns 1 if game over or 0 if game not over */
gint
BBreakoutTick (BBreakoutModule *pBreakoutModule)
{
  gint X, Y, X1, Y1, X2, Y2;

  /* player is dead */
  if (pBreakoutModule->DeadCnt >= 0)
  {
    /* decrement dead counter */
    pBreakoutModule->DeadCnt--;
    if (pBreakoutModule->DeadCnt == 0)
    {
      /* player has life left */
      if (pBreakoutModule->Lives > 0)
      {
        dbg_print ("BBreakout: player is alive again - %d live(s) left\n",
                   pBreakoutModule->Lives);
        /* initialize player */
        BBreakoutPlayerInit (pBreakoutModule);
      }

      /* game over */
      else
      {
        dbg_print ("BBreakout: player has no more lives left - game over\n");
	/* request end of game */
	b_module_request_stop ((BModule *) pBreakoutModule);
        /* return game over */
        return 1;
      }
    }

    /* output current picture */
    BBreakoutOutput (pBreakoutModule);

    /* return game not over */
    return 0;
  } /* if (pBreakoutModule->DeadCnt >= 0) */

  /* ball ist active */
  if (pBreakoutModule->BallActive)
  {
    /* bounce ball at left of game-field */
    if (pBreakoutModule->BallPosX <= 0
     && pBreakoutModule->BallDirX == -1)
      pBreakoutModule->BallDirX = 1;
    /* bounce ball at right of game-field */
    if (pBreakoutModule->BallPosX >= pBreakoutModule->SizeX - 1
     && pBreakoutModule->BallDirX == 1)
      pBreakoutModule->BallDirX = -1;
    /* bounce ball at top of game-field */
    if (pBreakoutModule->BallPosY <= 0
     && pBreakoutModule->BallDirY == -1)
      pBreakoutModule->BallDirY = 1;
    /* bounce ball at bottom of game-field */
    if (pBreakoutModule->BallPosY >= pBreakoutModule->SizeY - 1
     && pBreakoutModule->BallDirY == 1)
      pBreakoutModule->BallDirY = -1;

    /* bounce ball at edges of stones */
    for (Y = 0; Y < pBreakoutModule->StoneCntY; Y++)
    {
      for (X = 0; X < pBreakoutModule->StoneCntX; X++)
      {
        /* stone there */
        if (pBreakoutModule->ppStones[Y][X] > 0)
        {
          /* get position (all 4 corners) of stone */
          X1 = X * BBreakoutStoneSizeX;
          Y1 = Y * BBreakoutStoneSizeY;
          X2 = X1 + BBreakoutStoneSizeX - 1;
          Y2 = Y1 + BBreakoutStoneSizeY - 1;
          /* bounce ball at left edge of stone */
          if (pBreakoutModule->BallPosX == X1 - 1
           && pBreakoutModule->BallDirX == 1
           && pBreakoutModule->BallPosY >= Y1
           && pBreakoutModule->BallPosY <= Y2)
          {
            pBreakoutModule->BallDirX = -1; /* bounce ball */
            pBreakoutModule->ppStones[Y][X]--; /* stone has one hit less */
            if (pBreakoutModule->ppStones[Y][X] == 0) /* stone is gone */
              pBreakoutModule->StoneCnt--; /* one stone less */
          }
          /* bounce ball at right edge of stone */
          if (pBreakoutModule->BallPosX == X2 + 1
           && pBreakoutModule->BallDirX == -1
           && pBreakoutModule->BallPosY >= Y1
           && pBreakoutModule->BallPosY <= Y2)
          {
            pBreakoutModule->BallDirX = 1; /* bounce ball */
            pBreakoutModule->ppStones[Y][X]--; /* stone has one hit less */
            if (pBreakoutModule->ppStones[Y][X] == 0) /* stone is gone */
              pBreakoutModule->StoneCnt--; /* one stone less */
          }
          /* bounce ball at top edge of stone */
          if (pBreakoutModule->BallPosY == Y1 - 1
           && pBreakoutModule->BallDirY == 1
           && pBreakoutModule->BallPosX >= X1
           && pBreakoutModule->BallPosX <= X2)
          {
            pBreakoutModule->BallDirY = -1; /* bounce ball */
            pBreakoutModule->ppStones[Y][X]--; /* stone has one hit less */
            if (pBreakoutModule->ppStones[Y][X] == 0) /* stone is gone */
              pBreakoutModule->StoneCnt--; /* one stone less */
          }
          /* bounce ball at bottom edge of stone */
          if (pBreakoutModule->BallPosY == Y2 + 1
           && pBreakoutModule->BallDirY == -1
           && pBreakoutModule->BallPosX >= X1
           && pBreakoutModule->BallPosX <= X2)
          {
            pBreakoutModule->BallDirY = 1; /* bounce ball */
            pBreakoutModule->ppStones[Y][X]--; /* stone has one hit less */
            if (pBreakoutModule->ppStones[Y][X] == 0) /* stone is gone */
              pBreakoutModule->StoneCnt--; /* one stone less */
          }
        }
      } /* for (X ... */
    } /* for (Y ... */

    /* bounce ball at corners of stones */
    /* (it is important to bounce at all stone edges first) */
    for (Y = 0; Y < pBreakoutModule->StoneCntY; Y++)
    {
      for (X = 0; X < pBreakoutModule->StoneCntX; X++)
      {
        /* stone there */
        if (pBreakoutModule->ppStones[Y][X] > 0)
        {
          /* get position (all 4 corners) of stone */
          X1 = X * BBreakoutStoneSizeX;
          Y1 = Y * BBreakoutStoneSizeY;
          X2 = X1 + BBreakoutStoneSizeX - 1;
          Y2 = Y1 + BBreakoutStoneSizeY - 1;
          /* bounce ball at top-left corner of stone */
          if (pBreakoutModule->BallPosX == X1 - 1
           && pBreakoutModule->BallPosY == Y1 - 1
           && pBreakoutModule->BallDirX == 1
           && pBreakoutModule->BallDirY == 1)
          {
            pBreakoutModule->BallDirX = -1; /* bounce ball */
            pBreakoutModule->BallDirY = -1; /* bounce ball */
            pBreakoutModule->ppStones[Y][X]--; /* stone has one hit less */
            if (pBreakoutModule->ppStones[Y][X] == 0) /* stone is gone */
              pBreakoutModule->StoneCnt--; /* one stone less */
          }
          /* bounce ball at top-right corner of stone */
          if (pBreakoutModule->BallPosX == X2 + 1
           && pBreakoutModule->BallPosY == Y1 - 1
           && pBreakoutModule->BallDirX == -1
           && pBreakoutModule->BallDirY == 1)
          {
            pBreakoutModule->BallDirX = 1; /* bounce ball */
            pBreakoutModule->BallDirY = -1; /* bounce ball */
            pBreakoutModule->ppStones[Y][X]--; /* stone has one hit less */
            if (pBreakoutModule->ppStones[Y][X] == 0) /* stone is gone */
              pBreakoutModule->StoneCnt--; /* one stone less */
          }
          /* bounce ball at bottom-left corner of stone */
          if (pBreakoutModule->BallPosX == X1 - 1
           && pBreakoutModule->BallPosY == Y2 + 1
           && pBreakoutModule->BallDirX == 1
           && pBreakoutModule->BallDirY == -1)
          {
            pBreakoutModule->BallDirX = -1; /* bounce ball */
            pBreakoutModule->BallDirY = 1; /* bounce ball */
            pBreakoutModule->ppStones[Y][X]--; /* stone has one hit less */
            if (pBreakoutModule->ppStones[Y][X] == 0) /* stone is gone */
              pBreakoutModule->StoneCnt--; /* one stone less */
          }
          /* bounce ball at bottom-right corner of stone */
          if (pBreakoutModule->BallPosX == X2 + 1
           && pBreakoutModule->BallPosY == Y2 + 1
           && pBreakoutModule->BallDirX == -1
           && pBreakoutModule->BallDirY == -1)
          {
            pBreakoutModule->BallDirX = 1; /* bounce ball */
            pBreakoutModule->BallDirY = 1; /* bounce ball */
            pBreakoutModule->ppStones[Y][X]--; /* stone has one hit less */
            if (pBreakoutModule->ppStones[Y][X] == 0) /* stone is gone */
              pBreakoutModule->StoneCnt--; /* one stone less */
          }
	}
      } /* for (X ... */
    } /* for (Y ... */

    /* bounce ball at top edge of player */
    if (pBreakoutModule->BallPosY == pBreakoutModule->PlayerPosY - 1
     && pBreakoutModule->BallPosX >= pBreakoutModule->PlayerPosX
     && pBreakoutModule->BallPosX <= pBreakoutModule->PlayerPosX
                                   + pBreakoutModule->PlayerSizeX - 1
     && pBreakoutModule->BallDirY == 1)
    {
      pBreakoutModule->BallDirY = -1;
    }
    /* bounce ball at left corner of player */
    if (pBreakoutModule->BallPosX == pBreakoutModule->PlayerPosX - 1
     && pBreakoutModule->BallPosY == pBreakoutModule->PlayerPosY - 1
     && pBreakoutModule->BallDirX == 1
     && pBreakoutModule->BallDirY == 1)
    {
      pBreakoutModule->BallDirX = -1;
      pBreakoutModule->BallDirY = -1;
    }
    /* bounce ball at right corner of player */
    if (pBreakoutModule->BallPosX == pBreakoutModule->PlayerPosX
                                   + pBreakoutModule->PlayerSizeX
     && pBreakoutModule->BallPosY == pBreakoutModule->PlayerPosY - 1
     && pBreakoutModule->BallDirX == -1
     && pBreakoutModule->BallDirY == 1)
    {
      pBreakoutModule->BallDirX = 1;
      pBreakoutModule->BallDirY = -1;
    }

    /* move ball */
    pBreakoutModule->BallPosX += pBreakoutModule->BallDirX;
    pBreakoutModule->BallPosY += pBreakoutModule->BallDirY;

    /* all stones gone */
    if (pBreakoutModule->StoneCnt == 0)
    {
        dbg_print ("BBreakout: player has destroyed all stones - game over\n");
        /* output current picture */
        BBreakoutOutput (pBreakoutModule);
	/* request end of game */
	b_module_request_stop ((BModule *) pBreakoutModule);
        /* return game over */
        return 1;
    }

    /* player missed ball */
    if (pBreakoutModule->BallPosY >= pBreakoutModule->PlayerPosY)
    {
      /* decrement player's lives */
      pBreakoutModule->Lives--;
      dbg_print ("BBreakout: player missed the ball and died\n");
      /* player is dead now */
      pBreakoutModule->DeadCnt = BBreakoutDeadTime;
    }
  } /* if (pBreakoutModule->BallActive) */

  /* output current picture */
  BBreakoutOutput (pBreakoutModule);

  /* return game not over */
  return 0;
}

static void
b_breakout_module_class_init (BBreakoutModuleClass *klass)
{
  BModuleClass *module_class;

  module_class = B_MODULE_CLASS (klass);

  module_class->max_players = 1;

  module_class->query    = b_breakout_module_query;
  module_class->prepare  = b_breakout_module_prepare;
  module_class->relax    = b_breakout_module_relax;
  module_class->start    = b_breakout_module_start;
  module_class->event    = b_breakout_module_event;
  module_class->tick     = b_breakout_module_tick;
  module_class->describe = b_breakout_module_describe;
}

static void
b_breakout_module_init (BBreakoutModule *breakout_module)
{
  breakout_module->player_device_id = -1;
}

static gboolean
b_breakout_module_query (gint width,
                         gint height,
                         gint channels,
                         gint maxval)
{
  return (width >= BBreakoutSizeXMin && height >= BBreakoutSizeYMin
	  && channels == 1 && maxval + 1 >= BBreakoutColorCntMin);
}

static gboolean
b_breakout_module_prepare (BModule  *module,
                           GError  **error)
{
  gint SizePtrs, SizeRow, Size, Y;
  gchar *Ptr;

  BBreakoutModule *breakout_module = B_BREAKOUT_MODULE (module);

  /* initialize the module values that depend on the output device */
  breakout_module->MaxColor = module->maxval;                           /* maximum color value */
  breakout_module->FreeRows = module->height * 2 / 5;                   /* number of free rows over player */
  if (breakout_module->FreeRows < BBreakoutFreeRowsMin)
    breakout_module->FreeRows    = BBreakoutFreeRowsMin;
  breakout_module->StoneCntX     = module->width / BBreakoutStoneSizeX; /* number of stones in field */
  breakout_module->StoneCntY     = (module->height
                                    - breakout_module->FreeRows - 1)
                                   / BBreakoutStoneSizeY;
  breakout_module->StoneSizeX    = breakout_module->StoneCntX           /* size of stone area of game-field */
                                   * BBreakoutStoneSizeX;
  breakout_module->StoneSizeY    = breakout_module->StoneCntY
                                   * BBreakoutStoneSizeY;
  breakout_module->SizeX         = breakout_module->StoneSizeX;         /* size of game field */
  breakout_module->SizeY         = breakout_module->StoneSizeY
                                   + breakout_module->FreeRows + 1;
  breakout_module->OfsX          = (module->width                       /* offset of top left corner of game-field */
                                     - breakout_module->SizeX) / 2;
  breakout_module->OfsY          = (module->height
                                    - breakout_module->SizeY) / 2;
  breakout_module->PlayerSizeX   = breakout_module->SizeX / 3;          /* x-size and y-position of player */
  breakout_module->PlayerPosY    = breakout_module->SizeY - 1;
  breakout_module->PlayerPosXMax = breakout_module->SizeX               /* maximum x-position of player */
                                   - breakout_module->PlayerSizeX;
  breakout_module->PlayerStepX   = breakout_module->PlayerSizeX / 2;    /* x-step-size of player */

  /* allocate needed memory */
  SizePtrs = breakout_module->StoneCntY * sizeof (int *); /* get needed size */
  SizeRow = breakout_module->StoneCntX * sizeof (int);
  Size = SizePtrs + breakout_module->StoneCntY * SizeRow;
  Ptr = g_new (gchar, Size);

  breakout_module->ppStones = (int * *)Ptr; /* remember pointer */
  for (Y = 0; Y < breakout_module->StoneCntY; Y++)  /* generate pointers to rows */
    breakout_module->ppStones[Y] = (int *)(Ptr + SizePtrs + Y * SizeRow);

  return TRUE;
}

static void
b_breakout_module_relax (BModule  *module)
{
  BBreakoutModule *breakout_module = B_BREAKOUT_MODULE (module);

  g_free (breakout_module->ppStones);
  breakout_module->ppStones = NULL;
}

static void
b_breakout_module_start (BModule *module)
{
  /* set player's lives */
  ((BBreakoutModule *)module)->Lives = BBreakoutLives;

  /* start new game */
  BBreakoutNewGame ((BBreakoutModule *) module);

  /* start the tick machinery */
  b_module_ticker_start (module, BBreakoutTicks);
}

static void
b_breakout_module_event (BModule      *module,
                         BModuleEvent *event)
{
  BBreakoutModule *breakout;

  breakout = B_BREAKOUT_MODULE (module);

  switch (event->type)
    {
    case B_EVENT_TYPE_KEY:
      if (event->device_id == breakout->player_device_id)
        BBreakoutKey (breakout, event->key);
      break;

      case B_EVENT_TYPE_PLAYER_ENTERED:
      if (breakout->player_device_id == -1)
        {
          breakout->player_device_id = event->device_id;

          module->num_players++;
        }
      break;

    case B_EVENT_TYPE_PLAYER_LEFT:
      if (breakout->player_device_id == event->device_id)
        {
          breakout->player_device_id = -1;

          module->num_players--;
        }
      break;

    default:
      break;
    }
}

static gint
b_breakout_module_tick (BModule *module)
{
  gint GameOver;

  /* call tick-procedure */
  GameOver = BBreakoutTick ((BBreakoutModule *) module);

  /* we want to be called again in some milliseconds, if game is not over */
  if (GameOver)
    return 0;
  else
    return BBreakoutTicks;
}

static void
b_breakout_module_describe (BModule      *module,
                            const gchar **title,
                            const gchar **description,
                            const gchar **author)
{
  *title       = "BBreakout";
  *description = "Breakout game";
  *author      = "1stein";
}

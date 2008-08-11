/* BRetris: Tetris (from right to left) module for Blinkenlights
 *
 * Copyright (c) 2003 1stein <1stein@schuermans.info>
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

#define BRetrisVerMaj 1
#define BRetrisVerMin 0
#define BRetrisVerRev 2

/* minimum size and color count of display */
#define BRetrisSizeXMin 18
#define BRetrisSizeYMin 8
#define BRetrisMaxColorMin 1
/* speed of game (small number = fast) */
#define BRetrisSpeedMin 500
#define BRetrisSpeedMax 50
#define BRetrisSpeedInc 7 /* how fast to increase speed */
#define BRetrisSpeedDrop 20 /* speed for dropping stones */
/* the colors */
#define BRetrisColorEmpty(MaxColor)       (0)
#define BRetrisColorActiveStone(MaxColor) (MaxColor)
#define BRetrisColorStone(MaxColor)       ((MaxColor) * 3 / 4)
#define BRetrisColorRemoveStone(MaxColor) (MaxColor)

#define count(array)  (sizeof ((array))  / sizeof ((array)[0]))

#ifdef RETRIS_DEBUG
#define dbg_print g_print
#else
#define dbg_print(fmt, ...)
#endif

#define B_TYPE_RETRIS_MODULE         (b_type_retris_module)
#define B_RETRIS_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_RETRIS_MODULE, BRetrisModule))
#define B_RETRIS_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_RETRIS_MODULE, BRetrisModuleClass))
#define B_IS_RETRIS_MODULE(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_RETRIS_MODULE))

typedef struct _BRetrisModule BRetrisModule;
typedef struct _BRetrisModuleClass BRetrisModuleClass;


struct _BRetrisModule
{
  BModule parent_instance;

  gint SizeX, SizeY; /* size of game-field */
  gint MaxColor;     /* maximum color value */
  gint PlayerId;     /* device id of the player */
  gint Speed;        /* current speed */
  gint * * ppStones; /* two-dimensional array with stones [SizeX+4][SizeY] */
  gint CurStone;     /* number of current stone, -1 if none */
  gint CurX, CurY;   /* position of current stone */
  gint CurRot;       /* rotation of current stone */
  gint RemoveCol;    /* column being removed, -1 if none */
  gint Dropping;     /* flag if dropping current stone */
};

struct _BRetrisModuleClass
{
  BModuleClass parent_class;
};

static GType    b_retris_module_get_type   (GTypeModule         *module);
static void     b_retris_module_class_init (BRetrisModuleClass  *klass);
static void     b_retris_module_init       (BRetrisModule       *test_module);
static gboolean b_retris_module_query      (gint                 width,
                                            gint                 height,
                                            gint                 channels,
                                            gint                 maxval);
static gboolean b_retris_module_prepare    (BModule             *module,
                                            GError             **error);
static void     b_retris_module_relax      (BModule             *module);
static void     b_retris_module_start      (BModule             *module);
static void     b_retris_module_event      (BModule             *module,
                                            BModuleEvent        *event);
static gint     b_retris_module_tick       (BModule             *module);
static void     b_retris_module_describe   (BModule             *module,
                                            const gchar        **title,
                                            const gchar        **description,
                                            const gchar        **author);


static GType b_type_retris_module = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule * module)
{
  b_retris_module_get_type (module);

  return TRUE;
}

GType
b_retris_module_get_type (GTypeModule * module)
{
  if (!b_type_retris_module)
    {
      static const GTypeInfo retris_module_info = {
	sizeof (BRetrisModuleClass),
	NULL,			/* base_init */
	NULL,			/* base_finalize */
	(GClassInitFunc) b_retris_module_class_init,
	NULL,			/* class_finalize */
	NULL,			/* class_data */
	sizeof (BRetrisModule),
	0,			/* n_preallocs */
	(GInstanceInitFunc) b_retris_module_init,
      };

      /* !!!!!!!!! The name given in the next function MUST be unique! */

      b_type_retris_module = g_type_module_register_type (module,
							    B_TYPE_MODULE,
							    "BRetris",
							    &retris_module_info,
							    0);
    }

  return b_type_retris_module;
}

/* stones (always 4 lines with same stone in different rotation angles) */
typedef struct sBRetrisStonePos
{
  gint X, Y;
} stBRetrisStonePos;
typedef struct sBRetrisStoneRot
{
  stBRetrisStonePos Pos[4];
} stBRetrisStoneRot;
typedef struct sBRetrisStone
{
  stBRetrisStoneRot Rot[4];
} stBRetrisStone;
stBRetrisStone BRetrisStoneTable[] =
{
  { { { { { -2,  0 }, { -1,  0 }, {  0,  0 }, {  1,  0 } } }, /* the I */
      { { {  0, -2 }, {  0, -1 }, {  0,  0 }, {  0,  1 } } },
      { { { -2,  0 }, { -1,  0 }, {  0,  0 }, {  1,  0 } } },
      { { {  0, -2 }, {  0, -1 }, {  0,  0 }, {  0,  1 } } } } },
  { { { { {  1, -1 }, { -1,  0 }, {  0,  0 }, {  1,  0 } } }, /* the L */
      { { {  0, -1 }, {  0,  0 }, {  0,  1 }, {  1,  1 } } },
      { { { -1,  0 }, {  0,  0 }, {  1,  0 }, { -1,  1 } } },
      { { { -1, -1 }, {  0, -1 }, {  0,  0 }, {  0,  1 } } } } },
  { { { { { -1, -1 }, { -1,  0 }, {  0,  0 }, {  1,  0 } } }, /* the J */
      { { {  0, -1 }, {  1, -1 }, {  0,  0 }, {  0,  1 } } },
      { { { -1,  0 }, {  0,  0 }, {  1,  0 }, {  1,  1 } } },
      { { {  0, -1 }, {  0,  0 }, { -1,  1 }, {  0,  1 } } } } },
  { { { { {  0, -1 }, { -1,  0 }, {  0,  0 }, {  0,  1 } } }, /* the T */
      { { {  0, -1 }, { -1,  0 }, {  0,  0 }, {  1,  0 } } },
      { { {  0, -1 }, {  0,  0 }, {  1,  0 }, {  0,  1 } } },
      { { { -1,  0 }, {  0,  0 }, {  1,  0 }, {  0,  1 } } } } },
  { { { { {  0, -1 }, {  1, -1 }, {  0,  0 }, {  1,  0 } } }, /* the block */
      { { {  0, -1 }, {  1, -1 }, {  0,  0 }, {  1,  0 } } },
      { { {  0, -1 }, {  1, -1 }, {  0,  0 }, {  1,  0 } } },
      { { {  0, -1 }, {  1, -1 }, {  0,  0 }, {  1,  0 } } } } },
  { { { { {  0, -1 }, { -1,  0 }, {  0,  0 }, { -1,  1 } } }, /* the Z */
      { { { -1, -1 }, {  0, -1 }, {  0,  0 }, {  1,  0 } } },
      { { {  0, -1 }, { -1,  0 }, {  0,  0 }, { -1,  1 } } },
      { { { -1, -1 }, {  0, -1 }, {  0,  0 }, {  1,  0 } } } } },
  { { { { { -1, -1 }, { -1,  0 }, {  0,  0 }, {  0,  1 } } }, /* the S */
      { { {  0, -1 }, {  1, -1 }, { -1,  0 }, {  0,  0 } } },
      { { { -1, -1 }, { -1,  0 }, {  0,  0 }, {  0,  1 } } },
      { { {  0, -1 }, {  1, -1 }, { -1,  0 }, {  0,  0 } } } } },
};

/* check if a stone fits at a position */
/* retuns true if stone fits */
static gint
BRetrisCheckStone (BRetrisModule *pRetrisModule,
                   gint Stone, gint PosX, gint PosY, gint Rot)
{
  stBRetrisStoneRot * pStoneRot;
  stBRetrisStonePos * pStonePos;
  gint I, X, Y, SizeX, SizeY;

  /* check if stone is available */
  if (Stone < 0 || Stone >= count (BRetrisStoneTable))
    return 0;

  /* get size of stone array */
  SizeX = pRetrisModule->SizeX + 4;
  SizeY = pRetrisModule->SizeY;

  /* get pointer to stone rotation */
  pStoneRot = &BRetrisStoneTable[Stone]
               .Rot[Rot & 3];

  /* all 4 positions */
  for (I = 0; I < 4; I++)
  {
    /* get position */
    pStonePos = &pStoneRot->Pos[I];
    /* get coordinates */
    X = PosX + pStonePos->X;
    Y = PosY + pStonePos->Y;
    /* outside of array */
    if (X < 0 || X >= SizeX || Y < 0 || Y >= SizeY)
      return 0;
    /* position not free */
    if (pRetrisModule->ppStones[X][Y])
      return 0;
  }

  /* stone fits */
  return 1;
}

/* deactivate stone */
static void
BRetrisDeactivateStone (BRetrisModule *pRetrisModule)
{
  stBRetrisStoneRot * pStoneRot;
  stBRetrisStonePos * pStonePos;
  gint I, X, Y, SizeX, SizeY;

  /* check if stone is available */
  if (pRetrisModule->CurStone < 0 || pRetrisModule->CurStone >= count (BRetrisStoneTable))
    return;

  /* get size of stone array */
  SizeX = pRetrisModule->SizeX + 4;
  SizeY = pRetrisModule->SizeY;

  /* get pointer to stone rotation */
  pStoneRot = &BRetrisStoneTable[pRetrisModule->CurStone]
               .Rot[pRetrisModule->CurRot & 3];

  /* all 4 positions */
  for (I = 0; I < 4; I++)
  {
    /* get position */
    pStonePos = &pStoneRot->Pos[I];
    /* get coordinates */
    X = pRetrisModule->CurX + pStonePos->X;
    Y = pRetrisModule->CurY + pStonePos->Y;
    /* put stone into stone array */
    if (X >= 0 && X < SizeX && Y >= 0 && Y < SizeY)
      pRetrisModule->ppStones[X][Y] = 1;
  }
}

/* draw active stone */
static void
BRetrisDrawStone (BRetrisModule *pRetrisModule)
{
  stBRetrisStoneRot * pStoneRot;
  stBRetrisStonePos * pStonePos;
  gint I, X, Y, SizeX, SizeY;

  /* check if stone is available */
  if (pRetrisModule->CurStone < 0 || pRetrisModule->CurStone >= count (BRetrisStoneTable))
    return;

  /* get size of display */
  SizeX = pRetrisModule->SizeX;
  SizeY = pRetrisModule->SizeY;

  /* get pointer to stone rotation */
  pStoneRot = &BRetrisStoneTable[pRetrisModule->CurStone]
               .Rot[pRetrisModule->CurRot & 3];

  /* all 4 positions */
  for (I = 0; I < 4; I++)
  {
    /* get position */
    pStonePos = &pStoneRot->Pos[I];
    /* get coordinates */
    X = pRetrisModule->CurX + pStonePos->X;
    Y = pRetrisModule->CurY + pStonePos->Y;
    /* draw position */
    if (X >= 0 && X < SizeX && Y >= 0 && Y < SizeY)
      b_module_draw_point ((BModule *) pRetrisModule,
                           X, Y,
                           BRetrisColorActiveStone (pRetrisModule->MaxColor));
  }
}

/* output current picture */
static void
BRetrisOutput (BRetrisModule *pRetrisModule)
{
  gint X, Y;

  /* empty the screen */
  b_module_fill ((BModule *) pRetrisModule,
		 BRetrisColorEmpty (pRetrisModule->MaxColor));

  /* draw stones */
  for (X = 0; X < pRetrisModule->SizeX; X++)
  {
    for (Y = 0; Y < pRetrisModule->SizeY; Y++)
    {
      if (pRetrisModule->ppStones[X][Y])
      {
        if (X == pRetrisModule->RemoveCol)
          b_module_draw_point ((BModule *) pRetrisModule,
                               X, Y,
                               BRetrisColorRemoveStone (pRetrisModule->MaxColor));
        else
          b_module_draw_point ((BModule *) pRetrisModule,
                               X, Y,
                               BRetrisColorStone (pRetrisModule->MaxColor));
      }
    }
  }

  /* draw active stone */
  BRetrisDrawStone (pRetrisModule);

  /* update screen */
  b_module_paint ((BModule *) pRetrisModule);
}

/* start a new game */
static void
BRetrisNewGame (BRetrisModule *pRetrisModule)
{
  gint X, Y;

  /* set speed to minimal */
  pRetrisModule->Speed = BRetrisSpeedMin;

  /* remove all stones */
  for (X = 0; X < pRetrisModule->SizeX + 4; X++)
    for (Y = 0; Y < pRetrisModule->SizeY; Y++)
      pRetrisModule->ppStones[X][Y] = 0;

  /* no current stone yet */
  pRetrisModule->CurStone = -1;
  /* no column ist being removed */
  pRetrisModule->RemoveCol = -1;
  /* no stone is being dropped */
  pRetrisModule->Dropping = 0;

  dbg_print ("BRetris: new game\n");

  /* output current picture */
  BRetrisOutput (pRetrisModule);
}

/* key-procedure */
static void
BRetrisKey (BRetrisModule *pRetrisModule,
            BModuleKey     Key)
{
  switch (Key)
  {
    /* move stone up */
    case B_KEY_2:
      if (BRetrisCheckStone (pRetrisModule,
                             pRetrisModule->CurStone,
                             pRetrisModule->CurX,
                             pRetrisModule->CurY - 1,
                             pRetrisModule->CurRot))
      {
        pRetrisModule->CurY--;
        BRetrisOutput (pRetrisModule);
      }
      break;

    /* rotate stone */
    case B_KEY_3:
    case B_KEY_6:
      if (BRetrisCheckStone (pRetrisModule,
                             pRetrisModule->CurStone,
                             pRetrisModule->CurX,
                             pRetrisModule->CurY,
                             (pRetrisModule->CurRot + 1) & 3))
      {
        pRetrisModule->CurRot = (pRetrisModule->CurRot + 1) & 3;
        BRetrisOutput (pRetrisModule);
      }
      break;

    /* drop stone */
    case B_KEY_4:
      pRetrisModule->Dropping = 1; /* set flag for dropping stone */
      b_module_ticker_stop ((BModule *) pRetrisModule); /* restart the tick machinery */
      b_module_ticker_start ((BModule *) pRetrisModule, BRetrisSpeedDrop);
      break;

    /* move stone down */
    case B_KEY_8:
      if (BRetrisCheckStone (pRetrisModule,
                             pRetrisModule->CurStone,
                             pRetrisModule->CurX,
                             pRetrisModule->CurY + 1,
                             pRetrisModule->CurRot))
      {
        pRetrisModule->CurY++;
        BRetrisOutput (pRetrisModule);
      }
      break;

    /* rotate stone in other direction */
    case B_KEY_9:
      if (BRetrisCheckStone (pRetrisModule,
                             pRetrisModule->CurStone,
                             pRetrisModule->CurX,
                             pRetrisModule->CurY,
                             (pRetrisModule->CurRot + 3) & 3))
      {
        pRetrisModule->CurRot = (pRetrisModule->CurRot + 3) & 3;
        BRetrisOutput (pRetrisModule);
      }
      break;

    default:
      break;
  }
}

/* tick-procedure */
/* returns 1 if game over or 0 if game not over */
static gint
BRetrisTick (BRetrisModule *pRetrisModule)
{
  gint X, Y;

  /* dropping a stone */
  if (pRetrisModule->Dropping)
  {
    /* move stone one down */
    if (BRetrisCheckStone (pRetrisModule,
                           pRetrisModule->CurStone,
                           pRetrisModule->CurX - 1,
                           pRetrisModule->CurY,
                           pRetrisModule->CurRot))
      pRetrisModule->CurX--;
    /* stone has hit ground */
    else
      /* stop dropping stone */
      pRetrisModule->Dropping = 0;
  } /* dropping a stone */

  /* not dropping a stone */
  else
  {
    /* stone active */
    if (pRetrisModule->CurStone >= 0)
    {
      /* move stone one down */
      if (BRetrisCheckStone (pRetrisModule,
                             pRetrisModule->CurStone,
                             pRetrisModule->CurX - 1,
                             pRetrisModule->CurY,
                             pRetrisModule->CurRot))
        pRetrisModule->CurX--;
      /* stone has hit ground */
      else
      {
        BRetrisDeactivateStone (pRetrisModule); /* deactivate stone */
        pRetrisModule->CurStone = -1; /* no current stone */
        dbg_print ("BRetris: stone hit ground\n");
      }
    } /* stone active */

    /* no active stone */
    else
    {
      /* column is being removed */
      if (pRetrisModule->RemoveCol >= 0)
      {
        /* remove the column */
        for (X = pRetrisModule->RemoveCol; X < pRetrisModule->SizeX + 4 - 1; X++)
          for (Y = 0; Y < pRetrisModule->SizeY; Y++)
            pRetrisModule->ppStones[X][Y] = pRetrisModule->ppStones[X + 1][Y];
        dbg_print ("BRetris: column %d removed\n", pRetrisModule->RemoveCol);
        pRetrisModule->RemoveCol = -1; /* no column is being removed */
	/* increase speed */
        pRetrisModule->Speed = (pRetrisModule->Speed * (BRetrisSpeedInc - 1) + BRetrisSpeedMax) / BRetrisSpeedInc;
      } /* column is being removed */

      /* no column is being removed */
      else
      {
        /* check for a column to remove */
        for (X = 0; X < pRetrisModule->SizeX + 4; X++)
        {
          for (Y = 0; Y < pRetrisModule->SizeY; Y++)
            if (! pRetrisModule->ppStones[X][Y])
              break;
          if (Y >= pRetrisModule->SizeY) /* column is full */
            break;
        }

        /* found a column to remove */
        if (X < pRetrisModule->SizeX + 4)
        {
          pRetrisModule->RemoveCol = X;
          dbg_print ("BRetris: column %d is full\n", pRetrisModule->RemoveCol);
        }

        /* found no column to remove */
        else
        {
          /* check game over */
          for (X = pRetrisModule->SizeX; X < pRetrisModule->SizeX + 4; X++)
            for (Y = 0; Y < pRetrisModule->SizeY; Y++)
              if (pRetrisModule->ppStones[X][Y])
                /* found inactive stone right of display - game over */
                return 1;

          /* create new stone */
          pRetrisModule->CurStone = rand () % count (BRetrisStoneTable);
          pRetrisModule->CurX = pRetrisModule->SizeX + 2;
          pRetrisModule->CurY = pRetrisModule->SizeY / 2;
          pRetrisModule->CurRot = rand () & 3;
          dbg_print ("BRetris: created new stone\n");
        } /* found no column to remove */
      } /* no column is being removed */
    } /* no active stone */
  } /* not dropping a stone */

  /* output new picture */
  BRetrisOutput (pRetrisModule);

  /* return game not over */
  return 0;
}

static void
b_retris_module_class_init (BRetrisModuleClass *klass)
{
  BModuleClass *module_class;

  module_class = B_MODULE_CLASS (klass);

  module_class->query    = b_retris_module_query;
  module_class->prepare  = b_retris_module_prepare;
  module_class->relax    = b_retris_module_relax;
  module_class->start    = b_retris_module_start;
  module_class->event    = b_retris_module_event;
  module_class->tick     = b_retris_module_tick;
  module_class->describe = b_retris_module_describe;
}

static void
b_retris_module_init (BRetrisModule *retris_module)
{
}

static gboolean
b_retris_module_query (gint width,
                         gint height,
                         gint channels,
                         gint maxval)
{
  return (width >= BRetrisSizeXMin && height >= BRetrisSizeYMin
	  && channels == 1 && maxval >= BRetrisMaxColorMin);
}

static gboolean
b_retris_module_prepare (BModule  *module,
                           GError  **error)
{
  gint SizePtrs, SizeCol, Size, X;
  gchar *Ptr;

  BRetrisModule *pRetris_module = B_RETRIS_MODULE (module);

  /* initialize the module values that depend on the output device */
  pRetris_module->SizeX = module->width; /* size of game field */
  pRetris_module->SizeY = module->height;
  pRetris_module->MaxColor = module->maxval; /* maximum color value */
  pRetris_module->PlayerId = -1; /* no plyer in game yet */

  /* allocate needed memory */
  SizePtrs = (pRetris_module->SizeX + 4) * sizeof (int *); /* get needed size */
  SizeCol = pRetris_module->SizeY * sizeof (int);
  Size = SizePtrs + (pRetris_module->SizeX + 4) * SizeCol;
  Ptr = g_new (gchar, Size);

  pRetris_module->ppStones = (int * *)Ptr; /* remember pointer */
  for (X = 0, Ptr += SizePtrs; X < pRetris_module->SizeX + 4; X++, Ptr += SizeCol)  /* generate pointers to columns */
    pRetris_module->ppStones[X] = (int *)Ptr;

  return TRUE;
}

static void
b_retris_module_relax (BModule  *module)
{
  BRetrisModule *pRetris_module = B_RETRIS_MODULE (module);

  g_free (pRetris_module->ppStones);
  pRetris_module->ppStones = NULL;
}

static void
b_retris_module_start (BModule *module)
{
  /* start new game */
  BRetrisNewGame ((BRetrisModule *) module);

  /* start the tick machinery */
  b_module_ticker_start (module, BRetrisSpeedMin);
}

static void
b_retris_module_event (BModule      *module,
                       BModuleEvent *event)
{
  BRetrisModule *retris = B_RETRIS_MODULE (module);

  switch (event->type)
    {
    case B_EVENT_TYPE_KEY:
      BRetrisKey (retris, event->key);
      break;

    case B_EVENT_TYPE_PLAYER_ENTERED:
     if (retris->PlayerId == -1)
       {
         retris->PlayerId = event->device_id;
         module->num_players++;
       }
      break;

    case B_EVENT_TYPE_PLAYER_LEFT:
      if (retris->PlayerId == event->device_id)
        {
          retris->PlayerId = -1;
          module->num_players--;
        }
      break;

    default:
      break;
    }
}

static gint
b_retris_module_tick (BModule *module)
{
  gint GameOver;

  /* call tick-procedure */
  GameOver = BRetrisTick ((BRetrisModule *) module);

  /* game over */
  if (GameOver)
  {
    dbg_print ("BRetris: requesting stop\n");
    /* clear screen */
    b_module_fill (module, 0);
    b_module_paint (module);
    /* request end of game */
    b_module_request_stop (module);
    /* we do not want to be called again */
    return 0;
  }

  /* dropping a stone at the moment */
  if (((BRetrisModule *) module)->Dropping)
    /* we want to be called again in very few milliseconds */
    return BRetrisSpeedDrop;

  /* we want to be called again in some milliseconds */
  return ((BRetrisModule *) module)->Speed;
}

static void
b_retris_module_describe (BModule      *module,
                          const gchar **title,
                          const gchar **description,
                          const gchar **author)
{
  *title       = "BRetris";
  *description = "Tetris game from right to left";
  *author      = "1stein";
}


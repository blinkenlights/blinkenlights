/* BSitris4: Tetris (from sided to middle) module for Blinkenlights
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

#define BSitris4VerMaj 1
#define BSitris4VerMin 0
#define BSitris4VerRev 2

/* minimum size and color count of display */
#define BSitris4SizeXMin 26
#define BSitris4SizeYMin 6
#define BSitris4MaxColorMin 1
/* speed of game (small number = fast) */
#define BSitris4SpeedMin 500
#define BSitris4SpeedMax 50
#define BSitris4SpeedInc 7 /* how fast to increase speed */
#define BSitris4SpeedDrop 20 /* speed for dropping stones */
/* the colors */
#define BSitris4ColorEmpty(MaxColor)       (0)
#define BSitris4ColorActiveStone(MaxColor) (MaxColor)
#define BSitris4ColorStone(MaxColor)       ((MaxColor) * 3 / 4)
#define BSitris4ColorRemoveStone(MaxColor) (MaxColor)

#define count(array)  (sizeof ((array)) / sizeof ((array)[0]))

#ifdef SITRIS4_DEBUG
#define dbg_print g_print
#else
#define dbg_print(fmt, ...)
#endif

#define B_TYPE_SITRIS4_MODULE         (b_type_sitris4_module)
#define B_SITRIS4_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_SITRIS4_MODULE, BSitris4Module))
#define B_SITRIS4_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_SITRIS4_MODULE, BSitris4ModuleClass))
#define B_IS_SITRIS4_MODULE(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_SITRIS4_MODULE))

typedef struct _BSitris4Module BSitris4Module;
typedef struct _BSitris4ModuleClass BSitris4ModuleClass;


struct _BSitris4Module
{
  BModule parent_instance;

  gint RealSizeX, RealSizeY; /* real size of game-field */
  gint CenterX;              /* center of game-field (X direction) */
  gint LogSizeX, LogSizeY;   /* logical size of game-field */
  gint MaxColor;             /* maximum color value */
  gint PlayerId;             /* device id of the player */
  gint Speed;                /* current speed */
  gint * * ppStones;         /* two-dimensional array with stones [LogSizeX+4][LogSizeY] */
  gint CurStone;             /* number of current stone, -1 if none */
  gint CurX, CurY;           /* position of current stone */
  gint CurRot;               /* rotation of current stone */
  gint RemoveCol;            /* column being removed, -1 if none */
  gint Dropping;             /* flag if dropping current stone */
};

struct _BSitris4ModuleClass
{
  BModuleClass parent_class;
};

static GType    b_sitris4_module_get_type   (GTypeModule         *module);
static void     b_sitris4_module_class_init (BSitris4ModuleClass  *klass);
static void     b_sitris4_module_init       (BSitris4Module       *test_module);
static gboolean b_sitris4_module_query      (gint                 width,
                                            gint                 height,
                                            gint                 channels,
                                            gint                 maxval);
static gboolean b_sitris4_module_prepare    (BModule             *module,
                                            GError             **error);
static void     b_sitris4_module_relax      (BModule             *module);
static void     b_sitris4_module_start      (BModule             *module);
static void     b_sitris4_module_event      (BModule             *module,
                                            BModuleEvent        *event);
static gint     b_sitris4_module_tick       (BModule             *module);
static void     b_sitris4_module_describe   (BModule             *module,
                                            const gchar        **title,
                                            const gchar        **description,
                                            const gchar        **author);


static GType b_type_sitris4_module = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule * module)
{
  b_sitris4_module_get_type (module);

  return TRUE;
}

GType
b_sitris4_module_get_type (GTypeModule * module)
{
  if (!b_type_sitris4_module)
    {
      static const GTypeInfo sitris4_module_info = {
	sizeof (BSitris4ModuleClass),
	NULL,			/* base_init */
	NULL,			/* base_finalize */
	(GClassInitFunc) b_sitris4_module_class_init,
	NULL,			/* class_finalize */
	NULL,			/* class_data */
	sizeof (BSitris4Module),
	0,			/* n_preallocs */
	(GInstanceInitFunc) b_sitris4_module_init,
      };

      /* !!!!!!!!! The name given in the next function MUST be unique! */

      b_type_sitris4_module = g_type_module_register_type (module,
							    B_TYPE_MODULE,
							    "BSitris4",
							    &sitris4_module_info,
							    0);
    }

  return b_type_sitris4_module;
}

/* stones (always 4 lines with same stone in different rotation angles) */
typedef struct sBSitris4StonePos
{
  gint X, Y;
} stBSitris4StonePos;
typedef struct sBSitris4StoneRot
{
  stBSitris4StonePos Pos[4];
} stBSitris4StoneRot;
typedef struct sBSitris4Stone
{
  stBSitris4StoneRot Rot[4];
} stBSitris4Stone;
stBSitris4Stone BSitris4StoneTable[] =
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
BSitris4CheckStone (BSitris4Module *pSitris4Module,
                    gint Stone, gint PosX, gint PosY, gint Rot)
{
  stBSitris4StoneRot * pStoneRot;
  stBSitris4StonePos * pStonePos;
  gint I, X, Y, SizeX, SizeY;

  /* check if stone is available */
  if (Stone < 0 || Stone >= count (BSitris4StoneTable))
    return 0;

  /* get size of stone array */
  SizeX = pSitris4Module->LogSizeX + 4;
  SizeY = pSitris4Module->LogSizeY;

  /* get pointer to stone rotation */
  pStoneRot = &BSitris4StoneTable[Stone]
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
    if (pSitris4Module->ppStones[X][Y])
      return 0;
  }

  /* stone fits */
  return 1;
}

/* deactivate stone */
static void
BSitris4DeactivateStone (BSitris4Module *pSitris4Module)
{
  stBSitris4StoneRot * pStoneRot;
  stBSitris4StonePos * pStonePos;
  gint I, X, Y, SizeX, SizeY;

  /* check if stone is available */
  if (pSitris4Module->CurStone < 0
      || pSitris4Module->CurStone >= count (BSitris4StoneTable))
    return;

  /* get size of stone array */
  SizeX = pSitris4Module->LogSizeX + 4;
  SizeY = pSitris4Module->LogSizeY;

  /* get pointer to stone rotation */
  pStoneRot = &BSitris4StoneTable[pSitris4Module->CurStone]
               .Rot[pSitris4Module->CurRot & 3];

  /* all 4 positions */
  for (I = 0; I < 4; I++)
  {
    /* get position */
    pStonePos = &pStoneRot->Pos[I];
    /* get coordinates */
    X = pSitris4Module->CurX + pStonePos->X;
    Y = pSitris4Module->CurY + pStonePos->Y;
    /* put stone into stone array */
    if (X >= 0 && X < SizeX && Y >= 0 && Y < SizeY)
      pSitris4Module->ppStones[X][Y] = 1;
  }
}

/* draw active stone */
static void
BSitris4DrawStone (BSitris4Module *pSitris4Module)
{
  stBSitris4StoneRot * pStoneRot;
  stBSitris4StonePos * pStonePos;
  gint I, X, Y, CenterX, LogSizeX, LogSizeY;

  /* check if stone is available */
  if (pSitris4Module->CurStone < 0
      || pSitris4Module->CurStone >= count (BSitris4StoneTable))
    return;

  /* get center of display */
  CenterX = pSitris4Module->CenterX;
  /* get logical size of display */
  LogSizeX = pSitris4Module->LogSizeX;
  LogSizeY = pSitris4Module->LogSizeY;

  /* get pointer to stone rotation */
  pStoneRot = &BSitris4StoneTable[pSitris4Module->CurStone]
               .Rot[pSitris4Module->CurRot & 3];

  /* all 4 positions */
  for (I = 0; I < 4; I++)
  {
    /* get position */
    pStonePos = &pStoneRot->Pos[I];
    /* get coordinates */
    X = pSitris4Module->CurX + pStonePos->X;
    Y = pSitris4Module->CurY + pStonePos->Y;
    /* draw position */
    /* - game field is mirrored at center */
    /* - every field is two pixels wide */
    if (X >= 0 && X < LogSizeX && Y >= 0 && Y < LogSizeY)  {
      gint Color = BSitris4ColorActiveStone (pSitris4Module->MaxColor);
      b_module_draw_point ((BModule *) pSitris4Module,
                           CenterX - 2 * X - 2, Y,
                           Color);
      b_module_draw_point ((BModule *) pSitris4Module,
                           CenterX - 2 * X - 1, Y,
                           Color);
      b_module_draw_point ((BModule *) pSitris4Module,
                           CenterX + 2 * X, Y,
                           Color);
      b_module_draw_point ((BModule *) pSitris4Module,
                           CenterX + 2 * X + 1, Y,
                           Color);
    }
  }
}

/* output current picture */
static void
BSitris4Output (BSitris4Module *pSitris4Module)
{
  gint CenterX, X, Y;

  CenterX = pSitris4Module->CenterX;

  /* empty the screen */
  b_module_fill ((BModule *) pSitris4Module,
		 BSitris4ColorEmpty (pSitris4Module->MaxColor));

  /* draw stones */
  for (X = 0; X < pSitris4Module->LogSizeX; X++)
  {
    for (Y = 0; Y < pSitris4Module->LogSizeY; Y++)
    {
      if (pSitris4Module->ppStones[X][Y])
      {
        gint Color = X == pSitris4Module->RemoveCol
                  ? BSitris4ColorRemoveStone (pSitris4Module->MaxColor)
                  : BSitris4ColorStone (pSitris4Module->MaxColor);
        b_module_draw_point ((BModule *) pSitris4Module,
                               CenterX - 2 * X - 2, Y,
                               Color);
        b_module_draw_point ((BModule *) pSitris4Module,
                               CenterX - 2 * X - 1, Y,
                               Color);
        b_module_draw_point ((BModule *) pSitris4Module,
                               CenterX + 2 * X, Y,
                               Color);
        b_module_draw_point ((BModule *) pSitris4Module,
                               CenterX + 2 * X + 1, Y,
                               Color);
      }
    }
  }

  /* draw active stone */
  BSitris4DrawStone (pSitris4Module);

  /* update screen */
  b_module_paint ((BModule *) pSitris4Module);
}

/* start a new game */
static void
BSitris4NewGame (BSitris4Module *pSitris4Module)
{
  gint X, Y;

  /* set speed to minimal */
  pSitris4Module->Speed = BSitris4SpeedMin;

  /* remove all stones */
  for (X = 0; X < pSitris4Module->LogSizeX + 4; X++)
    for (Y = 0; Y < pSitris4Module->LogSizeY; Y++)
      pSitris4Module->ppStones[X][Y] = 0;

  /* no current stone yet */
  pSitris4Module->CurStone = -1;
  /* no column ist being removed */
  pSitris4Module->RemoveCol = -1;
  /* no stone is being dropped */
  pSitris4Module->Dropping = 0;

  dbg_print ("BSitris4: new game\n");

  /* output current picture */
  BSitris4Output (pSitris4Module);
}

/* key-procedure */
static void
BSitris4Key (BSitris4Module *pSitris4Module,
            BModuleKey     Key)
{
  switch (Key)
  {
    /* move stone up */
    case B_KEY_2:
      if (BSitris4CheckStone (pSitris4Module,
                             pSitris4Module->CurStone,
                             pSitris4Module->CurX,
                             pSitris4Module->CurY - 1,
                             pSitris4Module->CurRot))
      {
        pSitris4Module->CurY--;
        BSitris4Output (pSitris4Module);
      }
      break;

    /* rotate stone */
    case B_KEY_3:
    case B_KEY_6:
      if (BSitris4CheckStone (pSitris4Module,
                             pSitris4Module->CurStone,
                             pSitris4Module->CurX,
                             pSitris4Module->CurY,
                             (pSitris4Module->CurRot + 1) & 3))
      {
        pSitris4Module->CurRot = (pSitris4Module->CurRot + 1) & 3;
        BSitris4Output (pSitris4Module);
      }
      break;

    /* drop stone */
    case B_KEY_4:
      pSitris4Module->Dropping = 1;                       /* set flag for dropping stone */
      b_module_ticker_stop  ((BModule *) pSitris4Module); /* restart the tick machinery */
      b_module_ticker_start ((BModule *) pSitris4Module, BSitris4SpeedDrop);
      break;

    /* move stone down */
    case B_KEY_8:
      if (BSitris4CheckStone (pSitris4Module,
                             pSitris4Module->CurStone,
                             pSitris4Module->CurX,
                             pSitris4Module->CurY + 1,
                             pSitris4Module->CurRot))
      {
        pSitris4Module->CurY++;
        BSitris4Output (pSitris4Module);
      }
      break;

    /* rotate stone in other direction */
    case B_KEY_9:
      if (BSitris4CheckStone (pSitris4Module,
                             pSitris4Module->CurStone,
                             pSitris4Module->CurX,
                             pSitris4Module->CurY,
                             (pSitris4Module->CurRot + 3) & 3))
      {
        pSitris4Module->CurRot = (pSitris4Module->CurRot + 3) & 3;
        BSitris4Output (pSitris4Module);
      }
      break;

    default:
      break;
  }
}

/* tick-procedure */
/* returns 1 if game over or 0 if game not over */
static gint
BSitris4Tick (BSitris4Module *pSitris4Module)
{
  gint X, Y;

  /* dropping a stone */
  if (pSitris4Module->Dropping)
  {
    /* move stone one down */
    if (BSitris4CheckStone (pSitris4Module,
                           pSitris4Module->CurStone,
                           pSitris4Module->CurX - 1,
                           pSitris4Module->CurY,
                           pSitris4Module->CurRot))
      pSitris4Module->CurX--;
    /* stone has hit ground */
    else
      /* stop dropping stone */
      pSitris4Module->Dropping = 0;
  } /* dropping a stone */

  /* not dropping a stone */
  else
  {
    /* stone active */
    if (pSitris4Module->CurStone >= 0)
    {
      /* move stone one down */
      if (BSitris4CheckStone (pSitris4Module,
                             pSitris4Module->CurStone,
                             pSitris4Module->CurX - 1,
                             pSitris4Module->CurY,
                             pSitris4Module->CurRot))
        pSitris4Module->CurX--;
      /* stone has hit ground */
      else
      {
        BSitris4DeactivateStone (pSitris4Module); /* deactivate stone */
        pSitris4Module->CurStone = -1;            /* no current stone */
        dbg_print ("BSitris4: stone hit ground\n");
      }
    } /* stone active */

    /* no active stone */
    else
    {
      /* column is being removed */
      if (pSitris4Module->RemoveCol >= 0)
      {
        /* remove the column */
        for (X = pSitris4Module->RemoveCol; X < pSitris4Module->LogSizeX + 4 - 1; X++)
          for (Y = 0; Y < pSitris4Module->LogSizeY; Y++)
            pSitris4Module->ppStones[X][Y] = pSitris4Module->ppStones[X + 1][Y];
        dbg_print ("BSitris4: column %d removed\n", pSitris4Module->RemoveCol);
        pSitris4Module->RemoveCol = -1; /* no column is being removed */
	/* increase speed */
        pSitris4Module->Speed = (pSitris4Module->Speed * (BSitris4SpeedInc - 1)
                                 + BSitris4SpeedMax) / BSitris4SpeedInc;
      } /* column is being removed */

      /* no column is being removed */
      else
      {
        /* check for a column to remove */
        for (X = 0; X < pSitris4Module->LogSizeX + 4; X++)
        {
          for (Y = 0; Y < pSitris4Module->LogSizeY; Y++)
            if (! pSitris4Module->ppStones[X][Y])
              break;
          if (Y >= pSitris4Module->LogSizeY) /* column is full */
            break;
        }

        /* found a column to remove */
        if (X < pSitris4Module->LogSizeX + 4)
        {
          pSitris4Module->RemoveCol = X;
          dbg_print ("BSitris4: column %d is full\n", pSitris4Module->RemoveCol);
        }

        /* found no column to remove */
        else
        {
          /* check game over */
          for (X = pSitris4Module->LogSizeX; X < pSitris4Module->LogSizeX + 4; X++)
            for (Y = 0; Y < pSitris4Module->LogSizeY; Y++)
              if (pSitris4Module->ppStones[X][Y])
                /* found inactive stone right of display - game over */
                return 1;

          /* create new stone */
          pSitris4Module->CurStone = rand () % count (BSitris4StoneTable);
          pSitris4Module->CurX = pSitris4Module->LogSizeX + 2;
          pSitris4Module->CurY = pSitris4Module->LogSizeY / 2;
          pSitris4Module->CurRot = rand () & 3;
          dbg_print ("BSitris4: created new stone\n");
        } /* found no column to remove */
      } /* no column is being removed */
    } /* no active stone */
  } /* not dropping a stone */

  /* output new picture */
  BSitris4Output (pSitris4Module);

  /* return game not over */
  return 0;
}

static void
b_sitris4_module_class_init (BSitris4ModuleClass *klass)
{
  BModuleClass *module_class;

  module_class = B_MODULE_CLASS (klass);

  module_class->query    = b_sitris4_module_query;
  module_class->prepare  = b_sitris4_module_prepare;
  module_class->relax    = b_sitris4_module_relax;
  module_class->start    = b_sitris4_module_start;
  module_class->event    = b_sitris4_module_event;
  module_class->tick     = b_sitris4_module_tick;
  module_class->describe = b_sitris4_module_describe;
}

static void
b_sitris4_module_init (BSitris4Module *sitris4_module)
{
}

static gboolean
b_sitris4_module_query (gint width,
                         gint height,
                         gint channels,
                         gint maxval)
{
  return (width >= BSitris4SizeXMin && height >= BSitris4SizeYMin
	  && channels == 1 && maxval >= BSitris4MaxColorMin);
}

static gboolean
b_sitris4_module_prepare (BModule  *module,
                           GError  **error)
{
  gint SizePtrs, SizeCol, Size, X;
  gchar *Ptr;

  BSitris4Module *pSitris4_module = B_SITRIS4_MODULE (module);

  /* initialize the module values that depend on the output device */
  pSitris4_module->RealSizeX = module->width; /* size of game field */
  pSitris4_module->RealSizeY = module->height;
  pSitris4_module->CenterX = pSitris4_module->RealSizeX / 2;
  pSitris4_module->LogSizeX = pSitris4_module->RealSizeX / 4;
  pSitris4_module->LogSizeY = pSitris4_module->RealSizeY;
  pSitris4_module->MaxColor = module->maxval; /* maximum color value */
  pSitris4_module->PlayerId = -1; /* no plyer in game yet */

  /* allocate needed memory */
  SizePtrs = (pSitris4_module->LogSizeX + 4) * sizeof (int *); /* get needed size */
  SizeCol = pSitris4_module->LogSizeY * sizeof (int);
  Size = SizePtrs + (pSitris4_module->LogSizeX + 4) * SizeCol;
  Ptr = g_new (gchar, Size);

  pSitris4_module->ppStones = (int * *)Ptr; /* remember pointer */
  for (X = 0, Ptr += SizePtrs; /* generate pointers to columns */
       X < pSitris4_module->LogSizeX + 4;
       X++, Ptr += SizeCol)
    pSitris4_module->ppStones[X] = (int *)Ptr;

  return TRUE;
}

static void
b_sitris4_module_relax (BModule  *module)
{
  BSitris4Module *pSitris4_module = B_SITRIS4_MODULE (module);

  g_free (pSitris4_module->ppStones);
  pSitris4_module->ppStones = NULL;
}

static void
b_sitris4_module_start (BModule *module)
{
  /* start new game */
  BSitris4NewGame ((BSitris4Module *) module);

  /* start the tick machinery */
  b_module_ticker_start (module, BSitris4SpeedMin);
}

static void
b_sitris4_module_event (BModule      *module,
                       BModuleEvent *event)
{
  BSitris4Module *sitris4 = B_SITRIS4_MODULE (module);

  switch (event->type)
    {
    case B_EVENT_TYPE_KEY:
      BSitris4Key (sitris4, event->key);
      break;

    case B_EVENT_TYPE_PLAYER_ENTERED:
     if (sitris4->PlayerId == -1)
       {
         sitris4->PlayerId = event->device_id;
         module->num_players++;
       }
      break;

    case B_EVENT_TYPE_PLAYER_LEFT:
      if (sitris4->PlayerId == event->device_id)
        {
          sitris4->PlayerId = -1;
          module->num_players--;
        }
      break;

    default:
      break;
    }
}

static gint
b_sitris4_module_tick (BModule *module)
{
  gint GameOver;

  /* call tick-procedure */
  GameOver = BSitris4Tick ((BSitris4Module *) module);

  /* game over */
  if (GameOver)
  {
    dbg_print ("BSitris4: requesting stop\n");
    /* clear screen */
    b_module_fill (module, 0);
    b_module_paint (module);
    /* request end of game */
    b_module_request_stop (module);
    /* we do not want to be called again */
    return 0;
  }

  /* dropping a stone at the moment */
  if (((BSitris4Module *) module)->Dropping)
    /* we want to be called again in very few milliseconds */
    return BSitris4SpeedDrop;

  /* we want to be called again in some milliseconds */
  return ((BSitris4Module *) module)->Speed;
}

static void
b_sitris4_module_describe (BModule      *module,
                          const gchar **title,
                          const gchar **description,
                          const gchar **author)
{
  *title       = "BSitris4";
  *description = "Tetris game from the sides to the middle";
  *author      = "1stein";
}


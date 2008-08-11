/* BPacman: Pacman bluebox module for Blinkenlights
 *
 * Copyright (c) 2006 1stein <1stein@blinkenare.org>
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

/* #define PACMAN_DEBUG */

#include "config.h"

#include <string.h>
#include <stdlib.h>

#include <glib-object.h>
#include <gmodule.h>

#include <blib/blib.h>

/* color count of display */
#define BPacmanColorCntMin 4
/* number of lives for pacman */
#define BPacmanLives 3
/* speed of game */
#define BPacmanTicks 150
/* number of ticks pacman is dead after contact with monster */
#define BPacmanDeadTime 10
/* position of eaten monsters */
#define BPacmanMonsterNirvana -100
/* the colors */
#define BPacmanColorEmpty(MaxColor)        (0)
#define BPacmanColorWall(MaxColor)         ((MaxColor) * 2 / 3)
#define BPacmanColorPoint(MaxColor)        ((MaxColor) / 3)
#define BPacmanColorPacmanDead(MaxColor)   ((MaxColor) / 2)
#define BPacmanColorPacmanAlive(MaxColor)  (MaxColor)
#define BPacmanColorMonsterDead(MaxColor)  ((MaxColor) / 2)
#define BPacmanColorMonsterAlive(MaxColor) (MaxColor)
/* number of monsters */
#define BPacmanMonsterCntMax 5

/* meaning of game field data */
#define BPacmanFieldWall 1
#define BPacmanFieldChDir 2
#define BPacmanFieldPoint 4

/* the pacman game fields */
typedef struct BPacmanPosDirInfo
  {
    gint xPos, yPos; /* start position */
    gint xDir, yDir; /* start direction */
  } BPacmanPosDirInfo;
typedef struct BPacmanField
  {
    guint             xTotal, yTotal;                      /* total size of game filed */
    guint             xOffset, yOffset;                    /* offset of the visible part of the game field */
    guint             xVisible, yVisible;                  /* size of the visible part of the game field */
    gchar            *fieldData;                           /* the game field data */
    BPacmanPosDirInfo pacmanStart;                         /* start information of pacman */
    guint             monsterCnt;                          /* number of monsters */
    BPacmanPosDirInfo monsterStarts[BPacmanMonsterCntMax]; /* start information of monsters */
  } BPacmanField;
guchar BPacmanFieldDataArcade[] =
  {
    1,1,1,1,1,1,1,1,1,1,1,0,4,0,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,6,0,1,0,6,0,1,1,1,1,1,1,1,1,1,0,6,0,1,0,6,0,1,
    1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
    1,0,6,0,1,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,1,0,6,0,1,
    0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,
    4,0,6,0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,6,0,4,
    0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,0,
    1,0,6,0,1,0,6,0,6,0,6,0,1,0,6,0,6,0,6,0,1,0,6,0,1,
    1,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,1,
    1,0,6,0,1,0,6,0,1,0,6,0,6,0,6,0,1,0,6,0,1,0,6,0,1,
    1,0,0,0,1,0,0,0,1,0,0,0,0,0,0,0,1,0,0,0,1,0,0,0,1,
    1,0,6,0,1,0,6,0,1,1,1,1,1,1,1,1,1,0,6,0,1,0,6,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,0,4,0,1,1,1,1,1,1,1,1,1,1,1,
  };
guchar BPacmanFieldDataBluebox[] =
  {
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,1,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,1,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,1,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    4,0,6,0,1,1,1,1,1,1,1,0,6,0,1,1,1,1,1,1,1,1,1,1,1,0,6,0,1,0,6,0,1,1,1,1,1,1,1,1,1,0,6,0,1,1,1,1,1,1,1,1,1,0,6,0,1,1,1,1,1,1,1,1,1,0,6,0,1,0,6,0,1,1,1,1,1,1,1,1,1,1,1,0,6,0,1,1,1,1,1,1,1,0,6,0,4,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    1,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,1,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,1,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,6,0,1,
    1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,1,
    1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,
  };
BPacmanField BPacmanFields[] =
  {
    {
      .xTotal = 25, .yTotal = 19,
      .xOffset = 0, .yOffset = 0,
      .xVisible = 25, .yVisible = 19,
      .fieldData = BPacmanFieldDataArcade,
      .pacmanStart =
        {
          .xPos = -1, .yPos = 8,
          .xDir = 1, .yDir = 0,
        },
      .monsterCnt = 2,
      .monsterStarts =
        {
          {
            .xPos = 12, .yPos = -1,
            .xDir = 0, .yDir = 1,
          },
          {
            .xPos = 12, .yPos = 19,
            .xDir = 0, .yDir = -1,
          },
          {
            .xPos = -100, .yPos = -100,
            .xDir = -100, .yDir = -100,
          },
          {
            .xPos = -100, .yPos = -100,
            .xDir = -100, .yDir = -100,
          },
          {
            .xPos = -100, .yPos = -100,
            .xDir = -100, .yDir = -100,
          },
        },
    },
    {
      .xTotal = 97, .yTotal = 9,
      .xOffset = 0, .yOffset = 1,
      .xVisible = 97, .yVisible = 7,
      .fieldData = BPacmanFieldDataBluebox,
      .pacmanStart =
        {
          .xPos = -1, .yPos = 4,
          .xDir = 1, .yDir = 0,
        },
      .monsterCnt = 2,
      .monsterStarts =
        {
          {
            .xPos = 76, .yPos = 2,
            .xDir = -1, .yDir = 0,
          },
          {
            .xPos = 20, .yPos = 6,
            .xDir = -1, .yDir = 0,
          },
          {
            .xPos = -100, .yPos = -100,
            .xDir = -100, .yDir = -100,
          },
          {
            .xPos = -100, .yPos = -100,
            .xDir = -100, .yDir = -100,
          },
          {
            .xPos = -100, .yPos = -100,
            .xDir = -100, .yDir = -100,
          },
        },
    },
  };

#ifdef PACMAN_DEBUG
#define dbg_print g_print
#else
static inline void
dbg_print (char * Fmt, ...)
{
}
#endif

#define B_TYPE_PACMAN_MODULE         (b_type_pacman_module)
#define B_PACMAN_MODULE(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_PACMAN_MODULE, BPacmanModule))
#define B_PACMAN_MODULE_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_PACMAN_MODULE, BPacmanModuleClass))
#define B_IS_PACMAN_MODULE(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_PACMAN_MODULE))

typedef struct _BPacmanModule BPacmanModule;
typedef struct _BPacmanModuleClass BPacmanModuleClass;

struct _BPacmanModule
{
  BModule parent_instance;

  BPacmanField * field; 	                        /* pointer to information about the game field */
  gint OfsX, OfsY;					/* offset of top left corner of game-field */
  gint MaxColor;					/* maximum color value */
  gint * Points;					/* points in game field */
  gint PointsLeft;					/* numer of points left */
  BPacmanPosDirInfo pacman;				/* position and current direction of pacman */
  BPacmanPosDirInfo nextPacman;				/* next direction of pacman (ignore position in this structure) */
  gint MouthOpen;					/* pacman's mouth state */
  gint DeadCnt;						/* counter how long pacman is dead, 0 if not dead */
  BPacmanPosDirInfo monsters[BPacmanMonsterCntMax];	/* position and direction of monsters */
  gint MonstersDead;					/* !0 if monsters are dead */
  gint Lives;						/* rest of pacman's lives */
  gint player_device_id;
};

struct _BPacmanModuleClass
{
  BModuleClass parent_class;
};

static GType    b_pacman_module_get_type   (GTypeModule         *module);
static void     b_pacman_module_class_init (BPacmanModuleClass  *klass);
static void     b_pacman_module_init       (BPacmanModule       *test_module);
static gboolean b_pacman_module_query      (gint                 width,
                                            gint                 height,
                                            gint                 channels,
                                            gint                 maxval);
static gboolean b_pacman_module_prepare    (BModule             *module,
                                            GError             **error);
static void     b_pacman_module_start      (BModule             *module);
static void     b_pacman_module_stop       (BModule             *module);
static void     b_pacman_module_event      (BModule             *module,
                                            BModuleEvent        *event);
static gint     b_pacman_module_tick       (BModule             *module);
static void     b_pacman_module_describe   (BModule             *module,
                                            const gchar        **title,
                                            const gchar        **description,
                                            const gchar        **author);


static GType b_type_pacman_module = 0;


G_MODULE_EXPORT gboolean
b_module_register (GTypeModule * module)
{
  b_pacman_module_get_type (module);

  return TRUE;
}

GType
b_pacman_module_get_type (GTypeModule * module)
{
  if (!b_type_pacman_module)
    {
      static const GTypeInfo pacman_module_info = {
	sizeof (BPacmanModuleClass),
	NULL,			/* base_init */
	NULL,			/* base_finalize */
	(GClassInitFunc) b_pacman_module_class_init,
	NULL,			/* class_finalize */
	NULL,			/* class_data */
	sizeof (BPacmanModule),
	0,			/* n_preallocs */
	(GInstanceInitFunc) b_pacman_module_init,
      };

      /* !!!!!!!!! The name given in the next function MUST be unique! */

      b_type_pacman_module = g_type_module_register_type (module,
							  B_TYPE_MODULE,
							  "BPacman",
							  &pacman_module_info,
							  0);
    }

  return b_type_pacman_module;
}

/* select a game field based on the display size */
/* returns pointer to game field descriptor or NULL if no field has been found */
static BPacmanField * b_pacman_module_select_field (gint width, gint height)
{
  gint i;

  /* return pointer to descriptor of first game field that fits on the display */
  for (i = 0; i < sizeof (BPacmanFields)  / sizeof (BPacmanFields[0]); i++)
    if (width >= BPacmanFields[i].xVisible && height >= BPacmanFields[i].yVisible)
      return &BPacmanFields[i];

  /* no game fields fits on the display */
  return NULL;
}

/* check a pixel */
/* returns 1 if pixel in game field or 0 if it is not in game field */
static inline gint
BPacmanCheckPixel (BPacmanModule * pPacmanModule, gint X, gint Y)
{
  return (X >= 0 && X < pPacmanModule->field->xTotal
          && Y >= 0 && Y < pPacmanModule->field->yTotal);
}

/* check a position (for pacman or monster) */
/* returns 1 if position is free of walls or 0 if there is a wall at this position */
static gint
BPacmanCheckPos (BPacmanModule * pPacmanModule, gint X, gint Y)
{
  gint xTotal, XX, YY, iX, iY;

  xTotal = pPacmanModule->field->xTotal;
  for (YY = -1; YY <= 1; YY++)
    for (XX = -1; XX <= 1; XX++) {
      iX = X + XX;
      iY = Y + YY;
      if (BPacmanCheckPixel (pPacmanModule, iX, iY))
	if (pPacmanModule->field->fieldData[iY * xTotal + iX] & BPacmanFieldWall)
	  return 0;
    }
  return 1;
}

/* output a pixel */
static void
BPacmanOutputPixel (BPacmanModule *pPacmanModule, gint X, gint Y, gint Color)
{
  X -= pPacmanModule->field->xOffset;
  Y -= pPacmanModule->field->yOffset;
  if (X >= 0 && Y >= 0
      && X < pPacmanModule->field->xVisible
      && Y < pPacmanModule->field->yVisible)
    b_module_draw_point ((BModule *) pPacmanModule,
			 X + pPacmanModule->OfsX,
                         Y + pPacmanModule->OfsY,
			 Color);
}

/* output current picture */
static void
BPacmanOutput (BPacmanModule *pPacmanModule)
{
  gint xTotal, xOffset, yOffset, X, Y, I, Color;

  xTotal = pPacmanModule->field->xTotal;
  xOffset = pPacmanModule->field->xOffset;
  yOffset = pPacmanModule->field->yOffset;

  /* empty the screen */
  b_module_fill ((BModule *) pPacmanModule,
		 BPacmanColorEmpty (pPacmanModule->MaxColor));

  /* put walls and points onto screen */
  for (Y = 0; Y < pPacmanModule->field->yVisible; Y++)
    {
      for (X = 0; X < pPacmanModule->field->xVisible; X++)
	{
          gint i = (yOffset + Y) * xTotal + xOffset + X;
	  /* wall */
	  if (pPacmanModule->field->fieldData[i] & BPacmanFieldWall)
	    b_module_draw_point ((BModule *) pPacmanModule,
				 X + pPacmanModule->OfsX,
				 Y + pPacmanModule->OfsY,
				 BPacmanColorWall (pPacmanModule->MaxColor));
	  /* point */
	  if (pPacmanModule->Points[i])
	    b_module_draw_point ((BModule *) pPacmanModule,
				 X + pPacmanModule->OfsX,
				 Y + pPacmanModule->OfsY,
				 BPacmanColorPoint (pPacmanModule->MaxColor));
	}
    }

  /* get pacman color (dead or alive) */
  if (pPacmanModule->DeadCnt > 0)
    Color = BPacmanColorPacmanDead (pPacmanModule->MaxColor);
  else
    Color = BPacmanColorPacmanAlive (pPacmanModule->MaxColor);
  /* draw pacman */
  if (!pPacmanModule->MouthOpen)
    BPacmanOutputPixel (pPacmanModule, pPacmanModule->pacman.xPos,
			pPacmanModule->pacman.yPos, Color);
  if (!pPacmanModule->MouthOpen || pPacmanModule->pacman.xDir != -1)
    BPacmanOutputPixel (pPacmanModule, pPacmanModule->pacman.xPos - 1,
			pPacmanModule->pacman.yPos, Color);
  if (!pPacmanModule->MouthOpen || pPacmanModule->pacman.xDir != 1)
    BPacmanOutputPixel (pPacmanModule, pPacmanModule->pacman.xPos + 1,
			pPacmanModule->pacman.yPos, Color);
  if (!pPacmanModule->MouthOpen || pPacmanModule->pacman.yDir != -1)
    BPacmanOutputPixel (pPacmanModule, pPacmanModule->pacman.xPos,
			pPacmanModule->pacman.yPos - 1, Color);
  if (!pPacmanModule->MouthOpen || pPacmanModule->pacman.yDir != 1)
    BPacmanOutputPixel (pPacmanModule, pPacmanModule->pacman.xPos,
			pPacmanModule->pacman.yPos + 1, Color);

  /* get monster color (dead or alive) */
  if (pPacmanModule->MonstersDead)
    Color = BPacmanColorMonsterDead (pPacmanModule->MaxColor);
  else
    Color = BPacmanColorMonsterAlive (pPacmanModule->MaxColor);
  /* draw monsters */
  for (I = 0; I < pPacmanModule->field->monsterCnt; I++)
    {
      BPacmanOutputPixel (pPacmanModule, pPacmanModule->monsters[I].xPos,
			  pPacmanModule->monsters[I].yPos - 1, Color);
      BPacmanOutputPixel (pPacmanModule, pPacmanModule->monsters[I].xPos - 1,
			  pPacmanModule->monsters[I].yPos, Color);
      BPacmanOutputPixel (pPacmanModule, pPacmanModule->monsters[I].xPos,
			  pPacmanModule->monsters[I].yPos, Color);
      BPacmanOutputPixel (pPacmanModule, pPacmanModule->monsters[I].xPos + 1,
			  pPacmanModule->monsters[I].yPos, Color);
      BPacmanOutputPixel (pPacmanModule, pPacmanModule->monsters[I].xPos - 1,
			  pPacmanModule->monsters[I].yPos + 1, Color);
      BPacmanOutputPixel (pPacmanModule, pPacmanModule->monsters[I].xPos + 1,
			  pPacmanModule->monsters[I].yPos + 1, Color);
    }

  /* update screen */
  b_module_paint ((BModule *) pPacmanModule);
}

/* start a new game */
static void
BPacmanNewGame (BPacmanModule *pPacmanModule)
{
  gint xTotal, yTotal, X, Y, I;

  xTotal = pPacmanModule->field->xTotal;
  yTotal = pPacmanModule->field->yTotal;

  /* copy points into local points array and count them */
  pPacmanModule->PointsLeft = 0;
  for (Y = 0, I = 0; Y < yTotal; Y++)
    {
      for (X = 0; X < xTotal; X++, I++)
	{
          gint data = pPacmanModule->field->fieldData[I];
	  pPacmanModule->Points[I] = (data & BPacmanFieldPoint) != 0;
	  if (data & BPacmanFieldPoint)
	    pPacmanModule->PointsLeft++;
	}
    }

  /* set start position and direction of pacman */
  pPacmanModule->pacman = pPacmanModule->field->pacmanStart;
  pPacmanModule->nextPacman = pPacmanModule->pacman;
  pPacmanModule->MouthOpen = 0;
  pPacmanModule->DeadCnt = 0;
  /* set start position and direction of monsters */
  for (I = 0; I < pPacmanModule->field->monsterCnt; I++)
    pPacmanModule->monsters[I] = pPacmanModule->field->monsterStarts[I];
  pPacmanModule->MonstersDead = 0;

  dbg_print ("BPacman: new game\n");

  /* output current picture */
  BPacmanOutput (pPacmanModule);
}

/* key-procedure */
static void
BPacmanKey (BPacmanModule *pPacmanModule,
            BModuleKey     Key)
{
  switch (Key)
    {
    case B_KEY_2:
      /* set pacman's next direction to upwards */
      pPacmanModule->nextPacman.xDir = 0;
      pPacmanModule->nextPacman.yDir = -1;
      break;
    case B_KEY_4:
      /* set pacman's next direction to leftwards */
      pPacmanModule->nextPacman.xDir = -1;
      pPacmanModule->nextPacman.yDir = 0;
      break;
    case B_KEY_6:
      /* set pacman's next direction to rightwards */
      pPacmanModule->nextPacman.xDir = 1;
      pPacmanModule->nextPacman.yDir = 0;
      break;
    case B_KEY_8:
      /* set pacman's next direction to downwards */
      pPacmanModule->nextPacman.xDir = 0;
      pPacmanModule->nextPacman.yDir = 1;
      break;

    default:
      break;
    }

  /* output current picture */
  BPacmanOutput (pPacmanModule);
}

/* tick-procedure */
/* returns 1 if game over or 0 if game not over */
gint
BPacmanTick (BPacmanModule *pPacmanModule)
{
  gint X, Y, I, J, Left, Right, Up, Down, Cnt, DirX[4], DirY[4];

  gint xTotal = pPacmanModule->field->xTotal;

  /* toggle mouth state */
  pPacmanModule->MouthOpen = !pPacmanModule->MouthOpen;

  /* mouth now open */
  if (pPacmanModule->MouthOpen)
    {
      /* pacman is dead */
      if (pPacmanModule->DeadCnt > 0)
	{
	  /* decrement dead count */
	  pPacmanModule->DeadCnt--;
	  if (pPacmanModule->DeadCnt == 0)
          {
            /* another life */
            if (pPacmanModule->Lives > 0)
            {
              dbg_print ("BPacman: Pacman is alive again - %d live(s) left\n", pPacmanModule->Lives);
            }
            /* game over */
            else
            {
              dbg_print ("BPacman: Pacman has no lives left - game over\n");
              /* request end */
              b_module_request_stop ((BModule *)pPacmanModule);
	      /* do nothing else and return game over */
	      return 1;
            }
          }
	}
      /* pacman is alive */
      else
	{
	  /* pacman is at a position where changing direction is allowed */
	  if (BPacmanCheckPixel (pPacmanModule, pPacmanModule->pacman.xPos, pPacmanModule->pacman.yPos))
	    {
	      if (pPacmanModule->field->fieldData[pPacmanModule->pacman.yPos * pPacmanModule->field->xTotal
                                                + pPacmanModule->pacman.xPos] & BPacmanFieldChDir)
		{
		  /* change direction to new one */
		  pPacmanModule->pacman.xDir = pPacmanModule->nextPacman.xDir;
		  pPacmanModule->pacman.yDir = pPacmanModule->nextPacman.yDir;
		}
	    }

	  /* get new position (move in current direction) */
	  X = pPacmanModule->pacman.xPos + pPacmanModule->pacman.xDir;
	  Y = pPacmanModule->pacman.yPos + pPacmanModule->pacman.yDir;
	  /* check if there is a wall at the new position */
	  if (BPacmanCheckPos (pPacmanModule, X, Y))
	    {
	      /* set pacman to new position */
	      pPacmanModule->pacman.xPos = X;
	      pPacmanModule->pacman.yPos = Y;
	      /* wrap around */
	      if (pPacmanModule->pacman.xPos < -1 && pPacmanModule->pacman.xDir < 0)
		pPacmanModule->pacman.xPos = pPacmanModule->field->xTotal;
	      if (pPacmanModule->pacman.xPos > pPacmanModule->field->xTotal
		  && pPacmanModule->pacman.xDir > 0)
		pPacmanModule->pacman.xPos = -1;
	      if (pPacmanModule->pacman.yPos < -1 && pPacmanModule->pacman.yDir < 0)
		pPacmanModule->pacman.yPos = pPacmanModule->field->yTotal;
	      if (pPacmanModule->pacman.yPos > pPacmanModule->field->yTotal
		  && pPacmanModule->pacman.yDir > 0)
		pPacmanModule->pacman.yPos = -1;

	      /* point at this position */
	      if (BPacmanCheckPixel
		  (pPacmanModule, pPacmanModule->pacman.xPos, pPacmanModule->pacman.yPos))
		{
                  gint i = pPacmanModule->pacman.yPos * xTotal + pPacmanModule->pacman.xPos;
		  if (pPacmanModule->Points[i])
		    {
		      /* eat point */
		      pPacmanModule->Points[i] = 0;
		      pPacmanModule->PointsLeft--;
		      /* all points eaten */
		      if (pPacmanModule->PointsLeft == 0)
			{
			  /* monsters are dead now */
			  pPacmanModule->MonstersDead = 1;
			}
		    }
		}
	    }
	}

      /* monsters are alive */
      if (!pPacmanModule->MonstersDead)
	{
	  /* move monsters */
	  for (I = 0; I < pPacmanModule->field->monsterCnt; I++)
	    {
	      /* monster is at a position where changing direction is allowed */
	      if (BPacmanCheckPixel
		  (pPacmanModule,
		   pPacmanModule->monsters[I].xPos,
		   pPacmanModule->monsters[I].yPos))
		{
		  if (pPacmanModule->field->fieldData[pPacmanModule->monsters[I].yPos * xTotal
		      + pPacmanModule->monsters[I].xPos] & BPacmanFieldChDir)
		    {
		      /* get possible directions */
		      Left =
			BPacmanCheckPos (pPacmanModule,
					 pPacmanModule->monsters[I].xPos - 1,
					 pPacmanModule->monsters[I].yPos);
		      Right =
			BPacmanCheckPos (pPacmanModule,
					 pPacmanModule->monsters[I].xPos + 1,
					 pPacmanModule->monsters[I].yPos);
		      Up =
			BPacmanCheckPos (pPacmanModule,
					 pPacmanModule->monsters[I].xPos,
					 pPacmanModule->monsters[I].yPos - 1);
		      Down =
			BPacmanCheckPos (pPacmanModule,
					 pPacmanModule->monsters[I].xPos,
					 pPacmanModule->monsters[I].yPos + 1);
		      /* count directions */
		      Cnt = 0;
		      if (Left)
			Cnt++;
		      if (Right)
			Cnt++;
		      if (Up)
			Cnt++;
		      if (Down)
			Cnt++;
		      /* more than one direction possible */
		      if (Cnt > 1)
			{
			  /* exclude backward direction */
			  if (pPacmanModule->monsters[I].xDir == -1)
			    Right = 0;
			  if (pPacmanModule->monsters[I].xDir == 1)
			    Left = 0;
			  if (pPacmanModule->monsters[I].yDir == -1)
			    Down = 0;
			  if (pPacmanModule->monsters[I].yDir == 1)
			    Up = 0;
			}
		      /* put possible directions into an array */
		      Cnt = 0;
		      if (Left)
			{
			  DirX[Cnt] = -1;
			  DirY[Cnt] = 0;
			  Cnt++;
			}
		      if (Right)
			{
			  DirX[Cnt] = 1;
			  DirY[Cnt] = 0;
			  Cnt++;
			}
		      if (Up)
			{
			  DirX[Cnt] = 0;
			  DirY[Cnt] = -1;
			  Cnt++;
			}
		      if (Down)
			{
			  DirX[Cnt] = 0;
			  DirY[Cnt] = 1;
			  Cnt++;
			}
		      /* change direction to new one */
		      if (Cnt > 0)
			{
			  J = rand () % Cnt;	/* select one of the possible directions per coincidence */
			  pPacmanModule->monsters[I].xDir = DirX[J];
			  pPacmanModule->monsters[I].yDir = DirY[J];
			}
		    }
		}

	      /* get new position (move in current direction) */
	      X =
		pPacmanModule->monsters[I].xPos + pPacmanModule->monsters[I].xDir;
	      Y =
		pPacmanModule->monsters[I].yPos + pPacmanModule->monsters[I].yDir;
	      /* check if there is a wall at the new position */
	      if (BPacmanCheckPos (pPacmanModule, X, Y))
		{
		  /* set monster to new position */
		  pPacmanModule->monsters[I].xPos = X;
		  pPacmanModule->monsters[I].yPos = Y;
		  /* wrap around */
		  if (pPacmanModule->monsters[I].xPos < -1
		      && pPacmanModule->monsters[I].xDir < 0)
		    pPacmanModule->monsters[I].xPos = pPacmanModule->field->xTotal;
		  if (pPacmanModule->monsters[I].xPos > pPacmanModule->field->xTotal
		      && pPacmanModule->monsters[I].xDir > 0)
		    pPacmanModule->monsters[I].xPos = -1;
		  if (pPacmanModule->monsters[I].yPos < -1
		      && pPacmanModule->monsters[I].yDir < 0)
		    pPacmanModule->monsters[I].yPos = pPacmanModule->field->yTotal;
		  if (pPacmanModule->monsters[I].yPos > pPacmanModule->field->yTotal
		      && pPacmanModule->monsters[I].yDir > 0)
		    pPacmanModule->monsters[I].yPos = -1;
		}
	    }
	}
    }

  /* check if monster kills pacman / pacman kills monster */
  for (I = 0; I < pPacmanModule->field->monsterCnt; I++)
    {
      /* get distance */
      X = pPacmanModule->pacman.xPos - pPacmanModule->monsters[I].xPos;
      if (X < 0)
	X = -X;
      Y = pPacmanModule->pacman.yPos - pPacmanModule->monsters[I].yPos;
      if (Y < 0)
	Y = -Y;
      /* monster kills pacman / pacman kills monster */
      if (X + Y < 4)
	{
	  /* pacman kills monster */
	  if (pPacmanModule->MonstersDead)
	    {
	      /* move monster out of sight */
	      pPacmanModule->monsters[I].xPos = BPacmanMonsterNirvana;
	      pPacmanModule->monsters[I].yPos = BPacmanMonsterNirvana;
	      dbg_print ("BPacman: Pacman ate Monster\n");
	      /* check if all monsters are gone */
	      for (J = 0; J < pPacmanModule->field->monsterCnt; J++)
		if (pPacmanModule->monsters[J].xPos != BPacmanMonsterNirvana
		    || pPacmanModule->monsters[J].yPos != BPacmanMonsterNirvana)
		  break;
	      /* all monsters are gone */
	      if (J >= pPacmanModule->field->monsterCnt)
		{
		  dbg_print ("BPacman: Pacman has won game\n");
		  /* start new game */
		  BPacmanNewGame (pPacmanModule);
		}
	    }
	  /* monster kills pacman */
	  else
	    {
              /* decrease number of lives */
              if (pPacmanModule->DeadCnt == 0)
              {
                pPacmanModule->Lives--;
                dbg_print ("BPacman: Monster killed Pacman\n");
              }
              /* pacman is dead for the next few steps */
              pPacmanModule->DeadCnt = BPacmanDeadTime;
	    }
	}
    }

  /* output current picture */
  BPacmanOutput (pPacmanModule);

  /* return game not over */
  return 0;
}

static void
b_pacman_module_class_init (BPacmanModuleClass *klass)
{
  BModuleClass *module_class;

  module_class = B_MODULE_CLASS (klass);

  module_class->max_players = 1;

  module_class->query    = b_pacman_module_query;
  module_class->prepare  = b_pacman_module_prepare;
  module_class->start    = b_pacman_module_start;
  module_class->stop     = b_pacman_module_stop;
  module_class->event    = b_pacman_module_event;
  module_class->tick     = b_pacman_module_tick;
  module_class->describe = b_pacman_module_describe;
}

static void
b_pacman_module_init (BPacmanModule *pacman_module)
{
  pacman_module->player_device_id = -1;
}

static gboolean
b_pacman_module_query (gint width,
                       gint height,
                       gint channels,
                       gint maxval)
{
  return (b_pacman_module_select_field (width, height)
	  && channels == 1 && maxval + 1 >= BPacmanColorCntMin);
}

static gboolean
b_pacman_module_prepare (BModule  *module,
                         GError  **error)
{
  BPacmanModule *pacman_module = B_PACMAN_MODULE (module);

  /* select game field */
  pacman_module->field = b_pacman_module_select_field (module->width, module->height);
  if (pacman_module->field == NULL)
    pacman_module->field = &BPacmanFields[0];

  /* initialize the module values that depend on the output device */
  pacman_module->OfsX = (module->width - pacman_module->field->xVisible) / 2;	/* offset of top left corner of game-field */
  pacman_module->OfsY = (module->height - pacman_module->field->yVisible) / 2;
  pacman_module->MaxColor = module->maxval;	/* maximum color value */

  return TRUE;
}

static void
b_pacman_module_start (BModule *module)
{
  BPacmanModule * pPacmanModule = (BPacmanModule *)module;

  /* allocate array for points */
  pPacmanModule->Points = malloc (pPacmanModule->field->xTotal * pPacmanModule->field->yTotal * sizeof (int));

  /* set pacman's lives */
  ((BPacmanModule *)module)->Lives = BPacmanLives;

  /* start new game */
  BPacmanNewGame ((BPacmanModule *) module);

  /* start the tick machinery */
  b_module_ticker_start (module, BPacmanTicks);
}

static void
b_pacman_module_stop (BModule *module)
{
  BPacmanModule * pPacmanModule = (BPacmanModule *)module;

  /* free array for points */
  free (pPacmanModule->Points);
}

static void
b_pacman_module_event (BModule      *module,
                       BModuleEvent *event)
{
  BPacmanModule *pacman;

  pacman = B_PACMAN_MODULE (module);

  switch (event->type)
    {
    case B_EVENT_TYPE_KEY:
      if (event->device_id == pacman->player_device_id)
        BPacmanKey (pacman, event->key);
      break;

      case B_EVENT_TYPE_PLAYER_ENTERED:
      if (pacman->player_device_id == -1)
        {
          pacman->player_device_id = event->device_id;

          module->num_players++;
        }
      break;

    case B_EVENT_TYPE_PLAYER_LEFT:
      if (pacman->player_device_id == event->device_id)
        {
          pacman->player_device_id = -1;

          module->num_players--;
        }
      break;

    default:
      break;
    }
}

static gint
b_pacman_module_tick (BModule *module)
{
  gint GameOver;

  /* call tick-procedure */
  GameOver = BPacmanTick ((BPacmanModule *) module);

  /* we want to be called again in some milliseconds, if game is not over */
  if (GameOver)
    return 0;
  else
    return BPacmanTicks;
}

static void
b_pacman_module_describe (BModule      *module,
                          const gchar **title,
                          const gchar **description,
                          const gchar **author)
{
  *title       = "BPacman";
  *description = "Pacman game";
  *author      = "1stein";
}

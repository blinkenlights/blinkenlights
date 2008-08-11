/* DirectPong
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <directfb.h>

#include "fonts.h"
#include "gfx.h"
#include "gfx-house.h"
#include "gfx-simple.h"
#include "utils.h"


static IDirectFB            *dfb        = NULL;
static IDirectFBEventBuffer *keybuffer  = NULL;
static DFBResult             err        = DFB_OK;
static GfxEngine            *gfx        = NULL;
static Font                 *font       = NULL;

static int height = 0;

int
init_gfx (int         *argc,
          char       **argv[],
          int          rows,
          int          columns,
          const char  *engine)
{
  if (gfx != NULL)
    return 0;

  if (strcmp (engine, "simple") && strcmp (engine, "house"))
    return 0;

  DFBCHECK (DirectFBInit (argc, argv));

  /* create the super interface */
  DFBCHECK (DirectFBCreate (&dfb));
  DFBCHECK (dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN));

  DFBCHECK (dfb->CreateEventBuffer( dfb, DICAPS_KEYS, &keybuffer ));

  gfx = (GfxEngine *) malloc (sizeof (GfxEngine));
  gfx->width  = columns;
  gfx->height = rows;
  gfx->matrix = (unsigned char *) malloc (rows * columns);

  if (strcmp (engine, "simple") == 0)
    gfx_simple_create (gfx);
  else
    gfx_house_create (gfx);

  gfx->init (gfx, dfb);

  height = rows;

  font = font_load (DATADIR "numbers.pgm");

  return 1;
}

void
release_gfx (void)
{
  gfx->release (gfx);

  free (gfx->matrix);
  free (gfx);

  keybuffer->Release (keybuffer);
  dfb->Release (dfb);
}

void
draw_game_screen (int ball_x,
                  int ball_y,
                  int lpaddle,
                  int rpaddle)
{
  int width  = gfx->width;
  int height = gfx->height;
 
  memset (gfx->matrix, 0, width * height);

  /* a sanity check can't hurt */
  if (lpaddle >= 0 && lpaddle < height - 2)
    {
      lpaddle = lpaddle * width;
      gfx->matrix[lpaddle]             = 1;
      gfx->matrix[lpaddle +     width] = 1;
      gfx->matrix[lpaddle + 2 * width] = 1;
    }
  
  if (rpaddle >= 0 && rpaddle < height - 2)
    {
      rpaddle = rpaddle * width + (width - 1);
      gfx->matrix[rpaddle]             = 1;
      gfx->matrix[rpaddle +     width] = 1;
      gfx->matrix[rpaddle + 2 * width] = 1;
    }

  if (ball_x >= 0 && ball_x < width &&
      ball_y >= 0 && ball_y < height)
    gfx->matrix[ball_y * width + ball_x] = 1;

  gfx->draw (gfx);
}

int
draw_text_screen (const char *text)
{
  int            len;
  int            pos;
  int            i;
  int            x, y;
  char          *character;
  unsigned char *dest;

  memset (gfx->matrix, 0, gfx->width * gfx->height);

  if (!font)
    return 0;

  len = strlen (text);
  i = len * font->width + (len - 1) * font->spacing;
  x = (gfx->width - i) / 2;
  y = (gfx->height - font->height) / 2;

  if (x < 0 || x + i > gfx->width)
    return 0;
  if (y < 0 || y + font->height > gfx->height)
    return 0;

  dest = gfx->matrix + y * gfx->width + x;
 
  for (i = 0; i < len; i++)
    {
      character = strchr (font->chars, text[i]);
      if (character)
        {
          pos = character - font->chars;
          for (y = 0; y < font->height; y++)
            {
              memcpy (dest + y * gfx->width, 
                      font->data + pos * font->width + y * font->pitch,
                      font->width);
            }
          dest += font->width + font->spacing;        
        }
    }

  gfx->draw (gfx);
  
  return 1;
}

void
draw_empty_screen (void)
{
  memset (gfx->matrix, 0, gfx->width * gfx->height);
  
  gfx->draw (gfx);
}

int
check_events (int *lpaddle,
              int *rpaddle)
{
  DFBInputEvent evt;

  while (keybuffer->GetEvent( keybuffer, DFB_EVENT(&evt) ) == DFB_OK)
    {
      if (evt.type == DIET_KEYPRESS)
        {
          switch (evt.keycode)
            {
            case DIKC_ESCAPE:
            case DIKC_Q:
              return 1;
              
            case DIKC_SHIFT:
              if (lpaddle)
                *lpaddle = MAX ((*lpaddle)--, 0);
              break;
            case DIKC_CTRL:
              if (lpaddle) 
                *lpaddle = MIN ((*lpaddle)++, height - 3);
              break;
              
            case DIKC_UP:
              if (rpaddle) 
                *rpaddle = MAX ((*rpaddle)--, 0);
              break;
            case DIKC_DOWN:
              if (rpaddle) 
                *rpaddle = MIN ((*rpaddle)++, height - 3);
              break;
              
            default:
              break;
            }
        }
    }

  return 0;
}  

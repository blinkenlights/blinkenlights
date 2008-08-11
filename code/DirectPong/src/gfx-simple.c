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

#include <directfb.h>

#include "gfx.h"
#include "gfx-simple.h"
#include "utils.h"


static IDirectFBSurface *primary = NULL;
static IDirectFBSurface *area    = NULL;
static DFBResult         err     = DFB_OK;

static int block_size  = 0;


static void
gfx_simple_init (GfxEngine *engine,
                 void      *data)
{
  IDirectFB             *dfb;
  DFBSurfaceDescription  dsc;
  DFBRectangle           rect;
  int screen_width;
  int screen_height;

  dfb = (IDirectFB *) data;

  /* create the primary surface */
  dsc.flags = DSDESC_CAPS;
  dsc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;

  DFBCHECK (dfb->CreateSurface (dfb, &dsc, &primary));
  primary->GetSize (primary, &screen_width, &screen_height);

  primary->SetColor (primary, 0x66, 0x66, 0xCC, 0xFF);
  primary->FillRectangle (primary, 0, 0, screen_width, screen_height);
  primary->Flip (primary, NULL, 0);
  primary->FillRectangle (primary, 0, 0, screen_width, screen_height);

  block_size = MIN (screen_width / engine->width, 
                    screen_height / engine->height);

  rect.w = block_size * engine->width;
  rect.h = block_size * engine->height;
  rect.x = (screen_width  - rect.w) / 2;
  rect.y = (screen_height - rect.h) / 2;

  primary->GetSubSurface (primary, &rect, &area);
}

void
gfx_simple_release (GfxEngine *engine)
{
  area->Release (area);
  primary->Release (primary);
}

void
gfx_simple_draw (GfxEngine *engine)
{
  int x, y;
  unsigned char *matrix;

  matrix = engine->matrix;

  for (y = 0; y < engine->height; y++)
    {
      for (x = 0; x < engine->width; x++)
        {
          if (*matrix++)
            area->SetColor (area, 0xFF, 0xFF, 0xFF, 0xFF);
          else
            area->SetColor (area, 0x0, 0x0, 0x0, 0xFF);

          area->FillRectangle (area, 
                               x * block_size, y * block_size, 
                               block_size, block_size);
        }
    }

  primary->Flip (primary, NULL, DSFLIP_WAITFORSYNC);
}

void 
gfx_simple_create (GfxEngine *engine)
{
  engine->init    = gfx_simple_init;
  engine->release = gfx_simple_release;
  engine->draw    = gfx_simple_draw;  
}

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
#include "gfx-house.h"
#include "utils.h"


static IDirectFBSurface *primary = NULL;
static IDirectFBSurface *on      = NULL;
static IDirectFBSurface *off     = NULL;
static DFBResult         err     = DFB_OK;

static int offset_x = 0;
static int offset_y = 0;


static void
gfx_house_init (GfxEngine *engine,
                void      *data)
{
  IDirectFB              *dfb;
  IDirectFBImageProvider *provider;
  DFBSurfaceDescription   dsc;
  int screen_width;
  int screen_height;

  dfb = (IDirectFB *) data;

  /* try to set video mode, may fail */
  dfb->SetVideoMode (dfb, 800, 600, 16);

  /* create the primary surface */
  dsc.flags = DSDESC_CAPS;
  dsc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;

  DFBCHECK (dfb->CreateSurface (dfb, &dsc, &primary));
  primary->GetSize (primary, &screen_width, &screen_height);

  primary->SetColor (primary, 0x0, 0x0, 0x0, 0xFF);
  primary->FillRectangle (primary, 0, 0, screen_width, screen_height);
  primary->Flip (primary, NULL, 0);
  primary->FillRectangle (primary, 0, 0, screen_width, screen_height);

  /* load images */
  DFBCHECK (dfb->CreateImageProvider (dfb, 
                                      DATADIR "lights_on.png", &provider));
  DFBCHECK(provider->GetSurfaceDescription( provider, &dsc ));
  dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT;
  DFBCHECK(dfb->CreateSurface( dfb, &dsc, &on ));
  provider->RenderTo (provider, on, NULL);
  provider->Release (provider);

  DFBCHECK (dfb->CreateImageProvider (dfb, 
                                      DATADIR "lights_off.png", &provider));
  DFBCHECK(provider->GetSurfaceDescription( provider, &dsc ));
  dsc.flags = DSDESC_WIDTH | DSDESC_HEIGHT;
  DFBCHECK(dfb->CreateSurface( dfb, &dsc, &off ));
  provider->RenderTo (provider, off, NULL);
  provider->Release (provider);

  offset_x = (screen_width  - dsc.width)  / 2;
  offset_y = (screen_height - dsc.height) / 2;

  primary->Blit (primary, off, NULL, offset_x, offset_y);
  primary->Flip (primary, NULL, 0);
  primary->Blit (primary, off, NULL, offset_x, offset_y);
}

void
gfx_house_release (GfxEngine *engine)
{
  on->Release (on);
  off->Release (off);
  primary->Release (primary);
}

void
gfx_house_draw (GfxEngine *engine)
{
  DFBRectangle   rect;
  unsigned char *matrix;
  int x, y;

  matrix = engine->matrix;

  for (y = 0; y < engine->height; y++)
    {
      for (x = 0; x < engine->width; x++)
        {
          rect.x = 59 + x * 23;
          rect.y = 96 + y * 42;
          rect.w = 23;
          rect.h = 42;

          primary->Blit (primary, 
                         (*matrix++) ? on : off, 
                         &rect, offset_x + rect.x, offset_y + rect.y);
        }
    }

  primary->Flip (primary, NULL, DSFLIP_WAITFORSYNC);
}

void 
gfx_house_create (GfxEngine *engine)
{
  engine->init    = gfx_house_init;
  engine->release = gfx_house_release;
  engine->draw    = gfx_house_draw;  
}

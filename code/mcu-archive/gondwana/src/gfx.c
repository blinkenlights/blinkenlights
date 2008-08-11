/* gonwdwana - a simple bushfire simulator
 * Bushfire is a Blinkenlights Installation (TM)
 *
 * Copyright (c) 2002  Sven Neumann <sven@gimp.org>
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
#include <assert.h>

#include <directfb.h>

#include "magic-values.h"
#include "gfx.h"

#define DFBCHECK(x...)                                                     \
               err = x;                                                    \
               if (err != DFB_OK) {                                        \
                    fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
                    DirectFBErrorFatal( #x, err );                         \
               }

static void setup_luts (void);

static IDirectFB        *dfb           = NULL;
static IDirectFBSurface *primary       = NULL;
static DFBResult         err           = DFB_OK;
static DFBRectangle      rect          = { 0, 0, 0, 0 }; 
static int               screen_width  = 0;
static int               screen_height = 0;
static int               x0            = 0;
static int               y0            = 0;

static DFBColor          lut[HEIGHT][256];


IDirectFB *
setup_gfx (void)
{
  DFBSurfaceDescription dsc;

  if (dfb)
    return dfb;

  DirectFBSetOption ("no-cursor", NULL);
  DirectFBSetOption ("bg-color", "00000000");

  /* create the super interface */
  DFBCHECK (DirectFBCreate (&dfb));
  DFBCHECK (dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN));

  /* create the primary surface */
  dsc.flags = DSDESC_CAPS;
  dsc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;

  DFBCHECK (dfb->CreateSurface (dfb, &dsc, &primary));
  primary->GetSize (primary, &screen_width, &screen_height);

  primary->Clear (primary, 0, 0, 0, 0);
  primary->Flip (primary, NULL, 0);      
  primary->Clear (primary, 0, 0, 0, 0);

  rect.w = MIN (screen_width / WIDTH, screen_height / HEIGHT);
  rect.h = rect.w;

  x0 = (screen_width  - rect.w * WIDTH)  / 2;
  y0 = (screen_height - rect.h * HEIGHT) / 2;

  setup_luts ();

  return dfb;
}

void
close_gfx (void)
{
  primary->Release (primary);
  dfb->Release (dfb);
  dfb = NULL;
}

static void
hsv_to_rgb (unsigned char *hue,
            unsigned char *sat,
            unsigned char *val)
{
  double h, s, v;
  double f, p, q, t;

  if (*sat == 0)
    {
      *hue = *val;
      *sat = *val;
    }
  else
    {
      h = *hue * 6.0 / 255.0;
      s = *sat       / 255.0;
      v = *val       / 255.0;

      f = h - (int) h;
      p = v * (1.0 - s);
      q = v * (1.0 - (s * f));
      t = v * (1.0 - (s * (1.0 - f)));

      switch ((int) h)
        {
        case 0:
          *hue = v * 255.999;
          *sat = t * 255.999;
          *val = p * 255.999;
          break;

        case 1:
          *hue = q * 255.999;
          *sat = v * 255.999;
          *val = p * 255.999;
          break;

        case 2:
          *hue = p * 255.999;
          *sat = v * 255.999;
          *val = t * 255.999;
          break;

        case 3:
          *hue = p * 255.999;
          *sat = q * 255.999;
          *val = v * 255.999;
          break;

        case 4:
          *hue = t * 255.999;
          *sat = p * 255.999;
          *val = v * 255.999;
          break;

        case 5:
          *hue = v * 255.999;
          *sat = p * 255.999;
          *val = q * 255.999;
          break;
        }
    }
}

static void
setup_luts (void)
{
  int i;

  assert (HEIGHT == 3);

  for (i = 0; i < 256; i++)
    {
      lut[0][i].r = 42;
      lut[0][i].g = 0xFF;
      lut[0][i].b = i;
      hsv_to_rgb (&lut[0][i].r, &lut[0][i].g, &lut[0][i].b);

      lut[1][i].r = 21;
      lut[1][i].g = 0xFF;
      lut[1][i].b = i;
      hsv_to_rgb (&lut[1][i].r, &lut[1][i].g, &lut[1][i].b);

      lut[2][i].r = 0;
      lut[2][i].g = 0xFF;
      lut[2][i].b = i;
      hsv_to_rgb (&lut[2][i].r, &lut[2][i].g, &lut[2][i].b);
    }
}

void
output_frame (const unsigned char *state)
{
  int x, y;

  primary->Clear (primary, 0, 0, 0, 0);

  rect.y = y0;

  for (y = 0; y < HEIGHT; y++)
    {
      rect.x = x0;

      for (x = 0; x < WIDTH; x++)
        {
          DFBColor color = lut[y][*state];

          primary->SetColor (primary, color.r, color.g, color.b, 0xFF);
          primary->FillRectangle (primary, rect.x, rect.y, rect.w, rect.h);

          rect.x += rect.w;
          state++;
        }

      rect.y += rect.h;
    }

  primary->Flip (primary, NULL, DSFLIP_WAITFORSYNC);
}


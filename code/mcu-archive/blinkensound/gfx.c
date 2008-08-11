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

#include "values.h"
#include "gfx.h"

#define MIN(a,b) ((a) < (b) ? (a) : (b))

#define DFBCHECK(x...)                                                     \
               err = x;                                                    \
               if (err != DFB_OK) {                                        \
                    fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
                    DirectFBErrorFatal( #x, err );                         \
               }

static void setup_luts (void);

static IDirectFB            *dfb     = NULL;
static IDirectFBSurface     *primary = NULL;
static IDirectFBEventBuffer *events  = NULL;
static DFBResult       err           = DFB_OK;
static DFBRectangle    rect          = { 0, 0, 0, 0 }; 
static int             screen_width  = 0;
static int             screen_height = 0;
static int             text_x        = 0;
static int             text_y        = 0;
static int             x0            = 0;
static int             y0            = 0;

static DFBColor        lut[HEIGHT][256];


void
setup_gfx (int *argc, char **argv[])
{
  IDirectFBFont         *font;
  DFBSurfaceDescription  dsc;
  DFBFontDescription     font_dsc;
  
  if (dfb)
    return;

  DirectFBInit (argc, argv);

  DirectFBSetOption ("no-cursor", NULL);
  DirectFBSetOption ("bg-color", "00000000");

  DFBCHECK (DirectFBCreate (&dfb));
  DFBCHECK (dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN));

  dfb->CreateEventBuffer (dfb, DICAPS_KEYS, &events);

  dsc.flags = DSDESC_CAPS;
  dsc.caps = DSCAPS_PRIMARY | DSCAPS_FLIPPING;

  DFBCHECK (dfb->CreateSurface (dfb, &dsc, &primary));
  primary->GetSize (primary, &screen_width, &screen_height);

  font_dsc.flags  = DFDESC_HEIGHT;
  font_dsc.height = screen_width / 50;

  DFBCHECK (dfb->CreateFont (dfb,
                             "/usr/local/share/directfb-examples/fonts/decker.ttf",
                             &font_dsc, &font));

  primary->SetFont (primary, font);
  font->Release (font);

  primary->Clear (primary, 0, 0, 0, 0);
  primary->Flip (primary, NULL, DSFLIP_WAITFORSYNC);      
  primary->Clear (primary, 0, 0, 0, 0);

  rect.w = MIN (screen_width / WIDTH, screen_height / HEIGHT);
  rect.h = rect.w;

  x0 = (screen_width  - rect.w * WIDTH)  / 2;
  y0 = (screen_height - rect.h * HEIGHT) / 2;

  text_x = (screen_width - 20) / 8;
  text_y = screen_height - 20;

  setup_luts ();
}

void
close_gfx (void)
{
  if (!dfb)
    return;

  primary->Release (primary);
  if (events)
    events->Release (events);
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

static inline void
set_color (IDirectFBSurface *surface, int active)
{
  if (active)
    surface->SetColor (surface, 0xF0, 0xF0, 0xF0, 0xFF);
  else
    surface->SetColor (surface, 0x50, 0x50, 0x50, 0xFF);
}

void
gfx_show_frame (const unsigned char *data)
{
  static int last_mode  = -1;
  static int last_flags = -1;
  int x, y;

  if (!dfb)
    return;

  rect.y = y0;

  if (mode != last_mode || flags != last_flags)
    primary->Clear (primary, 0, 0, 0, 0xFF);

  for (y = 0; y < HEIGHT; y++)
    {
      rect.x = x0;

      for (x = 0; x < WIDTH; x++)
        {
          DFBColor color = lut[y][*data];

          primary->SetColor (primary, color.r, color.g, color.b, 0xFF);
          primary->FillRectangle (primary, rect.x, rect.y, rect.w, rect.h);

          rect.x += rect.w;
          data++;
        }

      rect.y += rect.h;
    }

  if (mode != last_mode || flags != last_flags)
    {
      int i;

      for (i = 0; i < 2; i++)
        {
          set_color (primary, flags & FLAG_PAUSE);
          primary->DrawString (primary, "Pause", -1,
                               10 + 0 * text_x, text_y, DSTF_LEFT);
          set_color (primary, flags & FLAG_CLEAR);
          primary->DrawString (primary, "Clear", -1,
                               10 + 1 * text_x, text_y, DSTF_LEFT);
          set_color (primary, flags & FLAG_FLIP);
          primary->DrawString (primary, "Flip", -1,
                               10 + 2 * text_x, text_y, DSTF_LEFT);
          set_color (primary, flags & FLAG_STEREO);
          primary->DrawString (primary, "Stereo", -1,
                               10 + 3 * text_x, text_y, DSTF_LEFT);
          set_color (primary, flags & FLAG_FOUNTAIN);
          primary->DrawString (primary, "Fountain", -1,
                               10 + 4 * text_x, text_y, DSTF_LEFT);
          set_color (primary, mode == MODE_VANALYZER);
          primary->DrawString (primary, "V-Analyzer", -1,
                               10 + 5 * text_x, text_y, DSTF_LEFT);
          set_color (primary, mode == MODE_HANALYZER);
          primary->DrawString (primary, "H-Analyzer", -1,
                               10 + 6 * text_x, text_y, DSTF_LEFT);
          set_color (primary, mode == MODE_VUMETER);
          primary->DrawString (primary, "VU-Meter", -1,
                               10 + 7 * text_x, text_y, DSTF_LEFT);
          
          if (i == 0)
            {
              primary->Flip (primary, NULL, DSFLIP_WAITFORSYNC);
              primary->Clear (primary, 0, 0, 0, 0xFF);
            }
        }

      last_mode  = mode;
      last_flags = flags;
    }
  else
    {
      primary->Flip (primary, NULL, DSFLIP_WAITFORSYNC);
    }
}

int
parse_keys (void)
{
  DFBInputEvent event;

  if (!events || events->HasEvent (events) != DFB_OK)
    return 0;

  while (events->GetEvent (events, DFB_EVENT(&event)) == DFB_OK)
    {
      if (event.type == DIET_KEYPRESS)
        {
        	switch (DFB_LOWER_CASE (event.key_symbol)) {
			case 'q':
			case DIKS_ESCAPE:
				close_gfx();
          			exit(0);
			break;
			case DIKS_F1:
				flags ^= FLAG_PAUSE;  
			break;
			case DIKS_F2:
				flags ^= FLAG_CLEAR;  
			break;
			case DIKS_F3:
				flags ^= FLAG_FLIP;  
			break;
			case DIKS_F4:
				flags ^= FLAG_STEREO; 
			break;
			case DIKS_F5:
				flags ^= FLAG_FOUNTAIN;
			break;
			case DIKS_F6:
				mode = MODE_VANALYZER;
			break;
			case DIKS_F7:
				mode = MODE_HANALYZER;
			break;
			case DIKS_F8:
				mode = MODE_VUMETER;
			break;
			default:
			break;
		}
        }
    }
  
  return 1;
}

/* 
 * Copyright (C) 2002  Sven Neumann <sven@gimp.org>
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

#include <stdlib.h>

#include <directfb.h>

#include "blipaqtypes.h"
#include "display.h"
#include "movie.h"
#include "progress.h"

#define HDL_OFFX   12
#define HDL_OFFY   30
#define HDL_DX     12
#define HDL_DY     22


static void  display_show_frame   (Display    *display,
                                   MovieFrame *frame);
static void  display_set_progress (Display    *display, 
                                   int         value);
static void  display_flip         (Display    *display);
static void  display_flip_left    (Display    *display);
static void  display_flip_right   (Display    *display);
static void  display_release      (Display    *display);


Display *
display_new (DisplayRotation  rotation)
{
  Display                *display;
  DFBSurfaceDescription   desc;
  DFBSurfacePixelFormat   pixelformat;
  DFBRectangle            rect;
  IDirectFB              *dfb;
  IDirectFBSurface       *surface;
  IDirectFBImageProvider *provider;  
  int                     width;
  int                     height;

  DirectFBSetOption ("no-cursor", NULL);

  if (DirectFBCreate (&dfb) != DFB_OK)
    return NULL;

  display = calloc (sizeof (Display), 1);
  display->Release   = display_release;

  display->dfb = dfb;
  dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN);

  /* create an event buffer */
  if (dfb->CreateEventBuffer (dfb, DICAPS_KEYS, &display->events) != DFB_OK)
    {
      display->Release (display);
      return NULL;
    }

  /* create the primary */
  desc.flags = DSDESC_CAPS;
  desc.caps = DSCAPS_PRIMARY;
  dfb->CreateSurface (dfb, &desc, &surface);

  surface->GetSize (surface, &width, &height);
  surface->GetPixelFormat (surface, &pixelformat);
  display->depth = DFB_BITS_PER_PIXEL (pixelformat);

  /* choose a subsurface in the middle */
  switch (rotation)
    {
    case ROTATE_NONE:
      display->Flip = display_flip;
      rect.w = 240;
      rect.h = 320;
      break;
    case ROTATE_LEFT:
      display->Flip = display_flip_left;
      rect.w = 320;
      rect.h = 240;
      break;
    case ROTATE_RIGHT:
      display->Flip = display_flip_right;
      rect.w = 320;
      rect.h = 240;
      break;
    }

  rect.x = (width  - rect.w) / 2;
  rect.y = (height - rect.h) / 2;

  surface->GetSubSurface (surface, &rect, &display->screen);
  surface->Release (surface);

  /* create a backbuffer */
  desc.flags = DSDESC_WIDTH | DSDESC_HEIGHT;
  desc.width  = 240;
  desc.height = 320;
  dfb->CreateSurface (dfb, &desc, &display->backbuffer);
  
  /* load the house image and blit it */
  if (dfb->CreateImageProvider (dfb, DATADIR "house.png", &provider) 
      != DFB_OK)
    {
      display->Release (display);
      return NULL;
    }
  provider->GetSurfaceDescription (provider, &desc);
  rect.x = 0;
  rect.y = 0;
  rect.w = desc.width;
  rect.h = desc.height;
  provider->RenderTo (provider, display->backbuffer, &rect);
  provider->Release (provider);
  
  /* create the progress bar */
  rect.x = 0;
  rect.y = desc.height;
  rect.w = desc.width;
  rect.h = 320 - desc.height;
  display->backbuffer->GetSubSurface (display->backbuffer, &rect, &surface);
  display->progress = progress_new (display, surface);
  surface->Release (surface);
  
  if (!display->progress)
    {
      display->Release (display);
      return NULL;
    }

  /* load the light image */
  if (dfb->CreateImageProvider (dfb, DATADIR "light.png", &provider) 
      != DFB_OK)
    {
      display->Release (display);
      return NULL;
    }
  provider->GetSurfaceDescription (provider, &desc);
  dfb->CreateSurface (dfb, &desc, &display->on);
  provider->RenderTo (provider, display->on, NULL);
  provider->Release (provider);
  
  dfb->CreateSurface (dfb, &desc, &display->off);
  rect.x = HDL_OFFX + HDL_DX;
  rect.y = HDL_OFFY + HDL_DY;
  rect.w = HDL_DX;
  rect.h = HDL_DY;
  display->off->Blit (display->off, display->backbuffer, &rect, 0, 0);

  display->ShowFrame   = display_show_frame;
  display->SetProgress = display_set_progress;

  display->SetProgress (display, 0);
  display->Flip (display);

  return display;
}

static void
display_show_frame (Display    *display, 
                    MovieFrame *frame)
{
  IDirectFBSurface *src;
  int               x, y;

  for (y = 0; y < 8; y++)
    for (x = 0; x < 18; x++)
      {
        src = (frame ? 
               (frame->data[y * 18 + x] ? display->on : display->off) :
               display->off);
        display->backbuffer->Blit (display->backbuffer, src, NULL,
                                   HDL_OFFX + x * HDL_DX, 
                                   HDL_OFFY + y * HDL_DY);
      }
}

static void
display_set_progress (Display *display,
                      int      value)
{
  display->progress->SetValue (display->progress, value);
}

static void
display_flip (Display *display)
{
  display->screen->Blit (display->screen, display->backbuffer, NULL, 0, 0);
}

#define ROTATE_LEFT(type)\
{\
  type d;\
  type s;\
  (__u8 *) dest += (240 - 1) * dest_pitch;\
  for (w = 0; w < 320; w++)\
    {\
      h = 240;\
      d = (type) dest + w;\
      s = src;\
      while (h--)\
        {\
          *d = *s;\
          s++;\
          (__u8*)d -= dest_pitch;\
        }\
      (__u8*)src += src_pitch;\
    }\
}

static void
display_flip_left (Display *display)
{
  void *src;
  void *dest;
  int   w, h;
  int   src_pitch, dest_pitch;

  display->backbuffer->Lock (display->backbuffer, DSLF_READ, &src, &src_pitch);
  display->screen->Lock (display->screen, DSLF_READ, &dest, &dest_pitch);

  switch (display->depth)
    {
    case 16:
      ROTATE_LEFT (__u16 *)
      break;
    case 32:
      ROTATE_LEFT (__u32 *)
      break;
    default:
      break;
    }

  display->backbuffer->Unlock (display->backbuffer);
  display->screen->Unlock (display->screen);
}

#define ROTATE_RIGHT(type)\
{\
  type d;\
  type s;\
  w = 320;\
  while (--w)\
    {\
      h = 240;\
      d = (type) dest + w;\
      s = src;\
      while (--h)\
        {\
          *d = *s;\
          s++;\
          (__u8*)d += dest_pitch;\
        }\
      (__u8*)src += src_pitch;\
    }\
}

static void
display_flip_right (Display *display)
{
  void *src;
  void *dest;
  int   w, h;
  int   src_pitch, dest_pitch;

  display->backbuffer->Lock (display->backbuffer, DSLF_READ, &src, &src_pitch);
  display->screen->Lock (display->screen, DSLF_READ, &dest, &dest_pitch);

  switch (display->depth)
    {
    case 16:
      ROTATE_RIGHT (__u16 *)
      break;
    case 32:
      ROTATE_RIGHT (__u32 *)
      break;
    default:
      break;
    }

  display->backbuffer->Unlock (display->backbuffer);
  display->screen->Unlock (display->screen);
}

static void
display_release (Display *display)
{
  if (display->progress)
    display->progress->Release (display->progress);
  if (display->events)
    display->events->Release (display->events);
  if (display->backbuffer)
    display->backbuffer->Release (display->backbuffer);
  if (display->screen)
    display->screen->Release (display->screen);
  if (display->on)
    display->on->Release (display->on);
  if (display->off)
    display->off->Release (display->off);
  if (display->dfb)
    display->dfb->Release (display->dfb);
  
  free (display);
}

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
#include "progress.h"


static void  progress_release   (Progress *progress);
static void  progress_set_value (Progress *progress,
                                 int       value);


Progress *
progress_new (Display          *display,
              IDirectFBSurface *surface)
{
  Progress               *progress;
  IDirectFBImageProvider *provider;  
  DFBSurfaceDescription   desc;

  progress = calloc (sizeof (Progress), 1);
  progress->Release  = progress_release;

  progress->surface = surface;
  surface->AddRef (surface);
  surface->GetSize (surface, &progress->width, &progress->height);
  
  /* load the progress images */
  if (display->dfb->CreateImageProvider (display->dfb, 
                                         DATADIR "progress.png", 
                                         &provider) != DFB_OK)
    {
      progress->Release (progress);
      return NULL;
    }
  provider->GetSurfaceDescription (provider, &desc);
  display->dfb->CreateSurface (display->dfb, &desc, &progress->bar);
  provider->RenderTo (provider, progress->bar, NULL);
  provider->Release (provider);
  
  if (display->dfb->CreateImageProvider (display->dfb, 
                                         DATADIR "thru.png", 
                                         &provider) != DFB_OK)
    {
      progress->Release (progress);
      return NULL;
    }
  provider->GetSurfaceDescription (provider, &desc);
  display->dfb->CreateSurface (display->dfb, &desc, &progress->thru);
  provider->RenderTo (provider, progress->thru, NULL);
  provider->Release (provider);

  progress->SetValue = progress_set_value;

  return progress;
}

static void
progress_release (Progress *progress)
{
  if (progress->surface)
    progress->surface->Release (progress->surface);
  if (progress->bar)
    progress->bar->Release (progress->bar);
  if (progress->thru)
    progress->thru->Release (progress->thru);

  free (progress);
}

static void
progress_set_value (Progress *progress,
                    int       value)
{
  DFBRegion  clip;

  clip.x1 = 0;
  clip.y1 = 0;
  clip.y2 = progress->height;
  clip.x2 = (value * progress->width) >> PROGRESS_SHIFT;

  if (clip.x2 > 0)
    {
      progress->surface->SetClip (progress->surface, &clip);
      progress->surface->TileBlit (progress->surface, 
                                   progress->bar, NULL, 0, 0);
     
      clip.x1 = clip.x2;
      clip.x2 = progress->width;
      progress->surface->SetClip (progress->surface, &clip);
      progress->surface->TileBlit (progress->surface, 
                                   progress->thru, NULL, 0, 0);
    }
  else
    {
      progress->surface->SetClip (progress->surface, NULL);
      progress->surface->TileBlit (progress->surface, 
                                   progress->thru, NULL, 0, 0);
    }
}

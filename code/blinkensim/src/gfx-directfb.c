/* blinkensim - a Blinkenlights simulator
 *
 * Copyright (c) 2001-2004  Sven Neumann <sven@gimp.org>
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

#include <blib/blib-directfb.h>

#include "gfx.h"


static gboolean   dfb_events_prepare   (GSource         *source,
                                        gint            *timeout);
static gboolean   dfb_events_check     (GSource         *source);
static gboolean   dfb_events_dispatch  (GSource         *source,
                                        GSourceFunc      callback,
                                        gpointer         user_data);


static  IDirectFB            *dfb       = NULL;
static  IDirectFBSurface     *primary   = NULL;
static  IDirectFBEventBuffer *events    = NULL;
static  GMainLoop            *main_loop = NULL;
static  gboolean              prepared  = FALSE;

static GSourceFuncs dfb_events_funcs =
{
  dfb_events_prepare,
  dfb_events_check,
  dfb_events_dispatch,
  NULL
};


gboolean
gfx_init (gint    *argc,
          gchar ***argv,
          GError **error)
{
  DFBResult  result;

  result = DirectFBInit (argc, argv);
  if (result != DFB_OK)
    {
      g_set_error (error, 0, 0, DirectFBErrorString (result));
      return FALSE;
    }

  return TRUE;
}

static gboolean
gfx_prepare (gint     width,
             gint     height,
             GError **error)
{
  DFBResult              result;
  DFBSurfaceDescription  dsc;

  if (prepared)
    return TRUE;

  result = DirectFBCreate (&dfb);
  if (result != DFB_OK)
    {
      g_set_error (error, 0, 0, DirectFBErrorString (result));
      return FALSE;
    }

  dfb->SetCooperativeLevel (dfb, DFSCL_FULLSCREEN);

  dfb->SetVideoMode (dfb, width, height, 32);

  dsc.flags = DSDESC_CAPS;
  dsc.caps  = DSCAPS_PRIMARY | DSCAPS_FLIPPING;;

  dfb->CreateSurface (dfb, &dsc, &primary);

#if (DIRECTFB_MAJOR_VERSION == 0 && \
     DIRECTFB_MINOR_VERSION == 9 && \
     DIRECTFB_MICRO_VERSION < 15)
  if (dfb->CreateEventBuffer (dfb, DICAPS_KEYS, &events) == DFB_OK)
#else
  if (dfb->CreateInputEventBuffer (dfb,
                                   DICAPS_KEYS, DFB_FALSE, &events) == DFB_OK)
#endif
    {
      GSource *source;

      source = g_source_new (&dfb_events_funcs, sizeof (GSource));
      g_source_set_priority (source, G_PRIORITY_DEFAULT);

      g_source_set_can_recurse (source, TRUE);
      g_source_attach (source, NULL);
    }

  prepared = TRUE;

  return prepared;
}

void
gfx_close (void)
{
  if (dfb)
    {
      events->Release (events);
      primary->Release (primary);
      dfb->Release (dfb);

      prepared = FALSE;
    }
}

GObject *
gfx_view_new (BTheme     *theme,
              GMainLoop  *loop,
              GError    **error)
{
  BViewDirectFB *view;

  if (!gfx_prepare (theme->width, theme->height, error))
    return FALSE;

  primary->Clear (primary,
                  theme->bg_color.r, theme->bg_color.g, theme->bg_color.b,
                  0xFF);
  primary->Flip (primary, NULL, 0);

  primary->Clear (primary,
                  theme->bg_color.r, theme->bg_color.g, theme->bg_color.b,
                  0xFF);

  view = b_view_directfb_new (theme, dfb, primary, error);
  if (! view)
    return NULL;

  primary->Flip (primary, NULL, DSFLIP_WAITFORSYNC);

  main_loop = loop;

  return G_OBJECT (view);
}

void
gfx_view_update (GObject      *view,
                 const guchar *data)
{
  b_view_directfb_update (B_VIEW_DIRECTFB (view), data);
  primary->Flip (primary, NULL, DSFLIP_WAITFORSYNC);
}


static gboolean
dfb_events_prepare (GSource *source,
                    gint    *timeout)
{
  gboolean retval = (events->HasEvent (events) == DFB_OK);

  *timeout = retval ? 10 : 50;

  return retval;
}

static gboolean
dfb_events_check (GSource *source)
{
  return (events->HasEvent (events) == DFB_OK);
}

static gboolean
dfb_events_dispatch (GSource     *source,
                     GSourceFunc  callback,
                     gpointer     user_data)
{
  DFBInputEvent event;

  while (events->GetEvent (events, DFB_EVENT (&event)) == DFB_OK)
    {
      if (event.type != DIET_KEYPRESS)
        continue;

      switch (DFB_LOWER_CASE (event.key_symbol))
        {
        case DIKS_EXIT:
        case DIKS_ESCAPE:
        case 'q':
          g_main_loop_quit (main_loop);
          return FALSE;
        }
    }

  return TRUE;
}

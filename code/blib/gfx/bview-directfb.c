/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2004  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
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

#include <directfb.h>

#include "blib/blib.h"

#include "bview-directfb.h"


static void  b_view_directfb_class_init (BViewDirectFBClass *class);
static void  b_view_directfb_init       (BViewDirectFB      *view);
static void  b_view_directfb_finalize   (GObject            *object);

static IDirectFBSurface * load_image    (BViewDirectFB      *view,
                                         IDirectFB          *dfb,
                                         const gchar        *filename);
static void          surface_release    (IDirectFBSurface   *surface);


static GObjectClass *parent_class = NULL;


GType
b_view_directfb_get_type (void)
{
  static GType view_type = 0;

  if (!view_type)
    {
      static const GTypeInfo view_info =
      {
        sizeof (BViewDirectFBClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_view_directfb_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BViewDirectFB),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_view_directfb_init,
      };

      view_type = g_type_register_static (G_TYPE_OBJECT,
                                          "BViewDirectFB", &view_info, 0);
    }

  return view_type;
}

static void
b_view_directfb_class_init (BViewDirectFBClass *class)
{
  GObjectClass *object_class;

  parent_class = g_type_class_peek_parent (class);
  object_class = G_OBJECT_CLASS (class);

  object_class->finalize = b_view_directfb_finalize;
}

static void
b_view_directfb_init (BViewDirectFB *view)
{
  view->theme  = NULL;
  view->images = g_hash_table_new_full (g_str_hash, g_str_equal,
                                        (GDestroyNotify) NULL,
                                        (GDestroyNotify) surface_release);
}

static void
b_view_directfb_finalize (GObject *object)
{
  BViewDirectFB *view;

  view = B_VIEW_DIRECTFB (object);

  if (view->dest)
    view->dest->Release (view->dest);

  if (view->theme)
    g_object_unref (view->theme);

  g_hash_table_destroy (view->images);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * b_view_directfb_new:
 * @theme: a #BTheme object
 * @dfb: the DirectFB super interface
 * @dest: the destination surface
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Creates a new #BViewDirectFB object suitable to display
 * Blinkenlights movies that fit the @theme. The view will render to
 * the @dest surface. If its size doesn't fit the screen size
 * specified in the theme, the view is drawn centered on the @dest
 * surface.
 *
 * Return value: a new #BViewDirectFB or %NULL in case of an error
 **/
BViewDirectFB *
b_view_directfb_new (BTheme            *theme,
                     IDirectFB         *dfb,
                     IDirectFBSurface  *dest,
                     GError           **error)
{
  BViewDirectFB *view;
  GList         *list;
  DFBRectangle   rect;

  g_return_val_if_fail (B_IS_THEME (theme), NULL);
  g_return_val_if_fail (dfb != NULL, NULL);
  g_return_val_if_fail (dest != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (theme->channels != 1)
    {
      g_set_error (error, 0, 0, "Channels != 1 is not (yet) supported");
      return NULL;
    }

  view = B_VIEW_DIRECTFB (g_object_new (B_TYPE_VIEW_DIRECTFB, NULL));

  view->theme = g_object_ref (theme);

  dest->Clear (dest,
               theme->bg_color.r, theme->bg_color.g, theme->bg_color.b, 0xFF);

  dest->GetSize (dest, &rect.w, &rect.h);

  rect.x = (rect.w - theme->width)  / 2;
  rect.y = (rect.h - theme->height) / 2;

  dest->GetSubSurface (dest, &rect, &view->dest);

  if (theme->bg_image)
    view->background = load_image (view, dfb, theme->bg_image);

  for (list = theme->overlays; list; list = list->next)
    {
      BOverlay *overlay = list->data;

      if (!overlay->image)
        continue;

      load_image (view, dfb, overlay->image);
    }

  b_view_directfb_update (view, NULL);

  return view;
}

/**
 * b_view_directfb_update:
 * @view: a #BViewDirectFB object
 * @frame_data: the frame data to display
 *
 * Displays a new frame on the @view. The @view expects @frame_data
 * in the range of 0 to 255.
 **/
void
b_view_directfb_update (BViewDirectFB *view,
                        const guchar  *frame_data)
{
  BTheme           *theme;
  IDirectFBSurface *dest;
  GList            *list;

  g_return_if_fail (B_IS_VIEW_DIRECTFB (view));
  g_return_if_fail (B_IS_THEME (view->theme));

  theme = view->theme;
  dest  = view->dest;

  dest->Clear (dest,
               theme->bg_color.r, theme->bg_color.g, theme->bg_color.b, 0xFF);

  if (view->background)
    {
      dest->SetBlittingFlags (dest, DSBLIT_NOFX);
      dest->Blit (dest,
                  view->background, NULL,
                  theme->bg_image_x, theme->bg_image_y);
    }

  if (!frame_data)
    return;

  for (list = theme->overlays; list; list = list->next)
    {
      BOverlay         *overlay = list->data;
      IDirectFBSurface *surface = NULL;
      GList            *windows;

      if (overlay->image)
        surface = g_hash_table_lookup (view->images, overlay->image);

      if (surface)
        {
          dest->SetBlittingFlags (dest, (DSBLIT_BLEND_ALPHACHANNEL |
                                         DSBLIT_BLEND_COLORALPHA));

          for (windows = overlay->windows; windows; windows = windows->next)
            {
              BWindow      *window = windows->data;
              DFBRectangle  rect;
              guchar        value;

              value = frame_data[(window->column +
                                  window->row * theme->columns)];
              if (!value)
                continue;

              window += (value * theme->maxval) / 256;

              rect.x = window->src_x;
              rect.y = window->src_y;
              rect.w = window->rect.w;
              rect.h = window->rect.h;

              dest->SetColor (dest,
                              0, 0, 0,
                              window->value == B_WINDOW_VALUE_ALL ?
                              value : 0xFF);
              dest->Blit (dest,
                          surface, &rect, window->rect.x, window->rect.y);
            }
        }
      else
        {
          dest->SetDrawingFlags (dest, DSDRAW_BLEND);

          for (windows = overlay->windows; windows; windows = windows->next)
            {
              BWindow *window = windows->data;
              guchar   value;

              value = frame_data[(window->column +
                                  window->row * theme->columns)];
              if (!value)
                continue;

              window += (value * theme->maxval) / 256;

              dest->SetColor (dest,
                              overlay->color.r,
                              overlay->color.g,
                              overlay->color.b,
                              (window->value == B_WINDOW_VALUE_ALL) ?
                              ((overlay->color.a + 1) * value) >> 8 :
                              overlay->color.a);
              dest->FillRectangle (dest,
                                   window->rect.x, window->rect.y,
                                   window->rect.w, window->rect.h);
            }
        }
    }
}

static IDirectFBSurface *
load_image (BViewDirectFB *view,
            IDirectFB     *dfb,
            const gchar   *filename)
{
  IDirectFBSurface       *surface;
  IDirectFBImageProvider *provider;
  DFBSurfaceDescription   dsc;
  DFBResult               err;

  surface = g_hash_table_lookup (view->images, filename);

  if (surface)
    return surface;

  err = dfb->CreateImageProvider (dfb, filename, &provider);

  if (err)
    {
      g_printerr ("Error loading image from file '%s': %s\n",
                  filename, DirectFBErrorString (err));
    }
  else
    {
      provider->GetSurfaceDescription (provider, &dsc);

      if (dfb->CreateSurface (dfb, &dsc, &surface) != DFB_OK)
        return NULL;

      provider->RenderTo (provider, surface, NULL);
      provider->Release (provider);
    }

  g_hash_table_insert (view->images, (gpointer) filename, surface);

  return surface;
}

static void
surface_release (IDirectFBSurface *surface)
{
  surface->Release (surface);
}

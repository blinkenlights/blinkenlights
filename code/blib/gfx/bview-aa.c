/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2003       pix <http://pix.test.at/>
 * Copyright (c) 2001-2002  The Blinkenlights Crew
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

#include <string.h> /* memset */

#include <aalib.h>

#include "blib/blib.h"

#include "bview-aa.h"


static void  b_view_aa_init (BViewAA *view);


GType
b_view_aa_get_type (void)
{
  static GType view_type = 0;

  if (!view_type)
    {
      static const GTypeInfo view_info =
      {
        sizeof (BViewAAClass),
        NULL,           /* base_init      */
        NULL,           /* base_finalize  */
        NULL,           /* class_init     */
        NULL,           /* class_finalize */
        NULL,           /* class_data     */
        sizeof (BViewAA),
        0,              /* n_preallocs    */
        (GInstanceInitFunc) b_view_aa_init,
      };

      view_type = g_type_register_static (G_TYPE_OBJECT,
                                          "BViewAA", &view_info, 0);
    }

  return view_type;
}

static void
b_view_aa_init (BViewAA *view)
{
  view->aac      = NULL;
  view->rows     = 0;
  view->columns  = 0;
  view->channels = 0;
}

/**
 * b_view_aa_new:
 * @aac: the AALIB context
 * @rows:
 * @columns:
 * @channels:
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Creates a new #BViewAA object suitable to display
 * Blinkenlights movies that fit the @theme. Most of the information
 * from the theme is ignored, except for the placement of the windows.
 *
 * Return value: a new #BViewAA or %NULL in case of an error
 **/
BViewAA *
b_view_aa_new (aa_context  *aac,
               gint         rows,
               gint         columns,
               gint         channels,
               GError     **error)
{
  BViewAA *view;

  g_return_val_if_fail (aac != NULL, NULL);
  g_return_val_if_fail (rows > 0 && columns > 0, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  if (channels != 1)
    {
      g_set_error (error, 0, 0, "Channels != 1 is not (yet) supported");
      return NULL;
    }

  view = B_VIEW_AA (g_object_new (B_TYPE_VIEW_AA, NULL));

  view->aac      = aac;
  view->rows     = rows;
  view->columns  = columns;
  view->channels = channels;

  /*
     work out how many pixels make up a screen character
     normally 2x2 but they threaten to change it to 3x3
   */
  view->char_width  = (aa_imgwidth (aac) / aa_scrwidth (aac));
  view->char_height = (aa_imgheight (aac) / aa_scrheight (aac));

  b_view_aa_update (view, NULL);

  return view;
}

/**
 * b_view_aa_update:
 * @view: a #BViewAA object
 * @frame_data: the frame data to display
 *
 * Displays a new frame on the @view. The @view expects @frame_data
 * in the range of 0 to 255.
 **/
void
b_view_aa_update (BViewAA      *view,
                  const guchar *frame_data)
{
  aa_context *aac;
  gint        x, y;

  g_return_if_fail (B_IS_VIEW_AA (view));

  aac = view->aac;

  /* clear the screen */
  memset (aa_image (aac), 0, aa_imgheight (aac) * aa_imgwidth (aac));

  if (!frame_data)
    return;

  for (y = 0; y < view->rows; y++)
    for (x = 0; x < view->columns; x++)
      {
        gint ix, iy;

        /* draw a block that fills one screen character */
        for (iy = 0; iy < view->char_height; iy++)
          for (ix = 0; ix < view->char_width; ix++)
            {
              aa_putpixel (aac,
                           x * view->char_width  + ix,
                           y * view->char_height + iy,
                           *frame_data);
            }

        frame_data++;
      }

  aa_fastrender (aac, 0, 0, view->columns, view->rows);

  aa_flush (aac);
}

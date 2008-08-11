/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
 *                          Michael Natterer <mitch@gimp.org>
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

#include <stdlib.h>
#include <string.h>

#include <glib-object.h>

#include "btypes.h"
#include "beffects.h"


static void   b_effects_init (BEffects *effects);


GType
b_effects_get_type (void)
{
  static GType effects_type = 0;

  if (!effects_type)
    {
      static const GTypeInfo effects_info =
      {
        sizeof (BEffectsClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	NULL,           /* class_init     */
	NULL,		/* class_finalize */
	NULL,		/* class_data     */
	sizeof (BEffects),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) b_effects_init,
      };

      effects_type = g_type_register_static (G_TYPE_OBJECT, 
                                             "BEffects", &effects_info, 0);
    }
  
  return effects_type;
}

static void
b_effects_init (BEffects *effects)
{
  effects->invert  = B_EFFECT_SCOPE_NONE;
  effects->vflip   = B_EFFECT_SCOPE_NONE;
  effects->hflip   = B_EFFECT_SCOPE_NONE;
  effects->lmirror = B_EFFECT_SCOPE_NONE;
  effects->rmirror = B_EFFECT_SCOPE_NONE;
}

/**
 * b_effects_new:
 * 
 * Creates a new #BEffects object with default values.
 * 
 * Return value: the newly allocate #BEffects object.
 **/
BEffects *
b_effects_new (void)
{
  return g_object_new (B_TYPE_EFFECTS, NULL);
}

/**
 * b_effects_apply:
 * @effects: a #BEffects object
 * @frame_data: data to apply the effects on
 * @width: width of the @frame_data buffer
 * @height: height of the @frame_data buffer
 * @channels: number of channels in the @frame_data buffer
 * @maxval: the maximum value for the @frame_data buffer
 * 
 * This function applies the effects described in the @effects object
 * to a single frame.
 **/
void
b_effects_apply (BEffects *effects,
                 guchar   *frame_data,
                 gint      width,
                 gint      height,
                 gint      channels,
                 gint      maxval)
{
  BEffectScope scope;
  gint         start_x;
  gint         end_x;
  gint         rowstride;

  g_return_if_fail (B_IS_EFFECTS (effects));
  g_return_if_fail (frame_data != NULL);
  g_return_if_fail (channels == 1);

  start_x   = 0;
  end_x     = width;
  rowstride = width;

  for (scope = B_EFFECT_SCOPE_ALL; scope >= B_EFFECT_SCOPE_LEFT; scope--)
    {
      if (scope == B_EFFECT_SCOPE_RIGHT)
        {
          width   /= 2;
          start_x  = width;
        }
      else if (scope == B_EFFECT_SCOPE_LEFT)
        {
          start_x = 0;
          end_x   = width;
        }

      if (effects->invert == scope)
        {
          gint    x, y;
          guchar *buf;

          buf = (guchar *) frame_data;

          for (y = 0; y < height; y++)
            {
              for (x = start_x; x < end_x; x++)
                {
                  buf[x] = maxval - buf[x];
                }

              buf += rowstride;
            }
        }

      if (effects->vflip == scope)
        {
          gint    y;
          guchar  temp[width];
          guchar *upper;
          guchar *lower;

          for (y = 0; y < height / 2; y++)
            {
              upper = frame_data + rowstride * y;
              lower = frame_data + rowstride * (height - y - 1);

              memcpy (temp, upper + start_x, width);
              memcpy (upper + start_x, lower + start_x, width);
              memcpy (lower + start_x, temp, width);
            }
        }

      if (effects->hflip == scope)
        {
          gint    x, y;
          guchar  temp;
          guchar *buf;

          buf = frame_data;

          for (y = 0; y < height; y++)
            {
              for (x = start_x; x < (start_x + width / 2); x++)
                {
                  gint swap_x;

                  swap_x = start_x + width - 1 - (x - start_x);

                  temp        = buf[x];
                  buf[x]      = buf[swap_x];
                  buf[swap_x] = temp;
                }

              buf += rowstride;
            }
        }

      if (effects->lmirror == scope || effects->rmirror == scope)
        {
          gint    x, y;
          guchar *buf;

          buf = frame_data;

          for (y = 0; y < height; y++)
            {
              for (x = start_x; x < (start_x + width / 2); x++)
                {
                  if (effects->lmirror == scope)
                    buf[width - 1 - x] = buf[x];
                  else if (effects->rmirror == scope)
                    buf[x] = buf[width - 1 - x];
                }

              buf += rowstride;
            }
        }
    }
}

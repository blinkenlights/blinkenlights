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

#include <stdlib.h>

#include "magic-values.h"
#include "effects.h"


void
apply_effects (unsigned char *frame,
               int            effect_invert,
               int            effect_vflip,
               int            effect_hflip,
               int            effect_lmirror,
               int            effect_rmirror)
{
  if (effect_invert)
    {
      int            x, y;
      unsigned char *buf;

      buf = (unsigned char *) frame;

      for (y = 0; y < HEIGHT; y++)
        {
          for (x = 0; x < WIDTH; x++)
            {
              *buf++ = 255 - *buf;
            } 
        }
    }

  if (effect_vflip)
    {
      unsigned char  temp[WIDTH];
      unsigned char *buf;

      buf = (unsigned char *) frame;

      memcpy (temp, buf, WIDTH);
      memcpy (buf, &buf[WIDTH * 2], WIDTH);
      memcpy (&buf[WIDTH * 2], temp, WIDTH);
    }

  if (effect_hflip)
    {
      int            x, y;
      unsigned char  temp;
      unsigned char *buf;

      buf = (unsigned char *) frame;

      for (y = 0; y < HEIGHT; y++)
        {
          for (x = 0; x < WIDTH / 2; x++)
            {
              temp               = buf[x];
              buf[x]             = buf[WIDTH - 1 - x];
              buf[WIDTH - 1 - x] = temp;
            }

          buf += WIDTH;
        }
    }

  if (effect_lmirror || effect_rmirror)
    {
      int            x, y;
      unsigned char *buf;

      buf = (unsigned char *) frame;

      for (y = 0; y < HEIGHT; y++)
        {
          for (x = 0; x < WIDTH / 2; x++)
            {
              if (effect_lmirror)
                buf[WIDTH - 1 - x] = buf[x];
              else if (effect_rmirror)
                buf[x] = buf[WIDTH - 1 - x];
            }

          buf += WIDTH;
        }
    }
}

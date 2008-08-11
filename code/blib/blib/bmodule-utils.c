/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
 *
 * Clipping functions and bresenham implementation based on code taken
 * from DirectFB (http://www.directfb.org/).
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

#include <glib-object.h>
#include <stdlib.h>
#include <string.h>

#include "btypes.h"
#include "bmodule.h"
#include "bmodule-utils.h"

typedef struct _BRegion BRegion;

struct _BRegion
{
  gint  x1;
  gint  y1;
  gint  x2;
  gint  y2;
};

#define SIGN(x)  ((x<0) ?  -1  :  ((x>0) ? 1 : 0))


static gboolean
clip_line (const BRegion *clip,
           BRegion       *line)
{
#define REGION_CODE(x,y,cx1,cx2,cy1,cy2) ( ( (y) > (cy2) ? 8 : 0) | \
                                           ( (y) < (cy1) ? 4 : 0) | \
                                           ( (x) > (cx2) ? 2 : 0) | \
                                           ( (x) < (cx1) ? 1 : 0) )

  guchar region_code1 = REGION_CODE (line->x1, line->y1,
                                     clip->x1, clip->x2, clip->y1, clip->y2);
  guchar region_code2 = REGION_CODE (line->x2, line->y2,
                                     clip->x1, clip->x2, clip->y1, clip->y2);
  
  while (region_code1 | region_code2)
    {
      if (region_code1 & region_code2)
        return FALSE;  /* line completely outside the clipping rectangle */

      if (region_code1)
        {
          if (region_code1 & 8)
            { /* divide line at bottom*/
              line->x1 = (line->x1 + (line->x2-line->x1) * 
                          (clip->y2 - line->y1) / (line->y2-line->y1));
              line->y1 = clip->y2;
            }
          else if (region_code1 & 4)
            { /* divide line at top*/
              line->x1 = (line->x1 + (line->x2-line->x1) *
                          (clip->y1 - line->y1) / (line->y2-line->y1));
              line->y1 = clip->y1;
            }
          else if (region_code1 & 2)
            { /* divide line at right*/
              line->y1 = (line->y1 +(line->y2-line->y1) *
                          (clip->x2 - line->x1) / (line->x2-line->x1));
              line->x1 = clip->x2;
            }
          else if (region_code1 & 1)
            { /* divide line at right*/
              line->y1 = (line->y1 +(line->y2-line->y1) *
                          (clip->x1 - line->x1) / (line->x2-line->x1));
              line->x1 = clip->x1;
            }
                         
          region_code1 = REGION_CODE (line->x1, line->y1,
                                      clip->x1, clip->x2, clip->y1, clip->y2);
        }
      else
        {
          if (region_code2 & 8)
            {  /* divide line at bottom*/
              line->x2 = (line->x1 + (line->x2-line->x1) *
                          (clip->y2 - line->y1) / (line->y2-line->y1));
              line->y2 = clip->y2;
            }
          else if (region_code2 & 4)
            { /* divide line at top*/
              line->x2 = (line->x1 +(line->x2-line->x1) *
                          (clip->y1 - line->y1) / (line->y2-line->y1));
              line->y2 = clip->y1;
            }
          else if (region_code2 & 2)
            { /* divide line at right*/
              line->y2 = (line->y1 +(line->y2-line->y1) *
                          (clip->x2 - line->x1) / (line->x2-line->x1));
              line->x2 = clip->x2;
            }
          else if (region_code2 & 1)
            { /* divide line at right*/
              line->y2 = (line->y1 +(line->y2-line->y1) * 
                          (clip->x1 - line->x1) / (line->x2-line->x1));
              line->x2 = clip->x1;
            }
        
          region_code2 = REGION_CODE (line->x2, line->y2,
                                      clip->x1, clip->x2, clip->y1, clip->y2);
        }
    }

  return TRUE;
}

/**
 * b_module_draw_point:
 * @module: a #BModule object
 * @x: x coordinate
 * @y: y coordinate
 * @value: the color to draw with
 * 
 * Draws a point to the @module's frame buffer. It is safe to specify
 * coordinates outside the buffer.
 **/
void
b_module_draw_point (BModule *module,
                     gint     x,
                     gint     y,
                     guchar   value)
{
  g_return_if_fail (B_IS_MODULE (module));

  if (x < 0 || x >= module->width)
    return;
  if (y < 0 || y >= module->height)
    return;

  module->buffer[module->width * y + x] = value;
}

/**
 * b_module_draw_line:
 * @module: a #BModule object
 * @x1: x coordinate of the start point
 * @y1: y coordinate of the start point
 * @x2: x coordinate of the end point
 * @y2: y coordinate of the end point
 * @value: the color to draw with
 * 
 * Draws a one-pixel wide line between two points to the module's
 * frame buffer. The endpoints are included in the line. If the line
 * exceeds the buffer, it is properly clipped.
 **/
void
b_module_draw_line (BModule *module,
                    gint     x1,
                    gint     y1,
                    gint     x2,
                    gint     y2,
                    guchar   value)
{
  BRegion  clip;
  BRegion  line;
  gint     x, y, i;
  gint     dx, dy, sdy;
  gint     dxabs, dyabs;
  guchar  *d;

  g_return_if_fail (B_IS_MODULE (module));

  clip.x1 = 0;
  clip.y1 = 0;
  clip.x2 = module->width  - 1;
  clip.y2 = module->height - 1;

  line.x1 = x1; 
  line.y1 = y1;  
  line.x2 = x2;
  line.y2 = y2;

  if (!clip_line (&clip, &line))
    return;

  /* the horizontal distance of the line */
  dx = line.x2 - line.x1;
  dxabs = abs (dx);

  /* the vertical distance of the line */
  dy = line.y2 - line.y1;
  dyabs = abs (dy);

  sdy = dx ? SIGN(dy) * SIGN(dx) : SIGN(dy);

  d = module->buffer;
      
  if (dx >= 0)
    d += line.y1 * module->width + line.x1;
  else
    d += line.y2 * module->width + line.x2;

  x = dyabs >> 1;
  y = dxabs >> 1;

  if (dxabs >= dyabs)
    { /* the line is more horizontal than vertical */
      for (i = 0; i <= dxabs; i++)
        {
          *d = value;

          d++;
          y += dyabs;
          if (y >= dxabs)
            {
              y -= dxabs;
              d += sdy * module->width;
            }
        }
    }
  else /* the line is more vertical than horizontal */
    {
      for (i = 0; i <= dyabs; i++)
        {
          *d = value;
          d += sdy * module->width;

          x += dxabs;       
          if (x >= dyabs)
            {
              x -= dyabs;
              d++;
            }
        }
    }
}

/**
 * b_module_fill:
 * @module: a #BModule object
 * @value: the color to draw with
 * 
 * Fills the module's framebuffer with @value.
 **/
void
b_module_fill (BModule *module,
               guchar   value)
{
  g_return_if_fail (B_IS_MODULE (module));

  memset (module->buffer, value, module->width * module->height);
}

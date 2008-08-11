/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include "config.h"

#include <glib.h>
#include <string.h>

#include "bsound.h"

void
apply_voodoo (void)
{
  gint x, y, v;
  gboolean done[WIDTH];
  guchar _audio_vals[WIDTH];
  guchar audio_min[WIDTH];
  guchar audio_max[WIDTH];
  gulong foo;
  
  for (x = 0; x < WIDTH; x++)
     {
	foo = 0;
	audio_min[x] = 0xff;
	audio_max[x] = 0;
	
	for (y = 0; y < AUDIO_RBUF_SIZE; y++)
	   {
	     foo += audio_vals[y][x];

	     if (audio_vals[y][x] < audio_min[x])
		     audio_min[x] = audio_vals[y][x];
	     
	     if (audio_vals[y][x] > audio_max[x])
		     audio_max[x] = audio_vals[y][x];
	     
	    }
	
	foo /= AUDIO_RBUF_SIZE;
	_audio_vals[x] = foo;
     }
  
  memset (matrix, 0, WIDTH*HEIGHT);
  memset (done, 0, WIDTH);
  
  for (x = 0; x < WIDTH; x++)
    {
       _audio_vals[x] -= audio_min[x];
       _audio_vals[x] = (int) ((float) _audio_vals[x] * 255.0 / (float) audio_max[x]);
       v = (((float) (0xff - _audio_vals[x]) * HEIGHT) / (float) 0xff);
       for (y = HEIGHT-1; y > v; y--)
 	 {
	   matrix[y][x] = 0xff - (v - y) * 15;
#if 0
	  if (0xff - _audio_vals[x] > (int) (((float) y / (float) HEIGHT) * (float) 0xff) && !done[x])
	    {
	       matrix[y][x] = 0xff - (HEIGHT - y) * 6;
	       done[x] = TRUE;
	    }
#endif
	  }
       
       
       if (audio_peak_color[x] > 0)
         audio_peak_color[x]--;

       if (v <= audio_peak[x])
         {
	   audio_peak[x] = v;
	   audio_peak_timeout[x] = AUDIO_PEAK_TIMEOUT;
	   audio_peak_color[x] = 0xff - (HEIGHT - v) * 4;
	 }
       if (audio_peak_timeout[x]-- <= 0)
         {
  	   audio_peak_timeout[x] = AUDIO_PEAK_TIMEOUT;
	   if (audio_peak[x] < HEIGHT-1)
	     audio_peak[x]++;
	  }
       
       matrix[audio_peak[x]][x] = audio_peak_color[x];
    }
}


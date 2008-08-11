/* DirectPong
 *
 * Copyright (c) 2001  Sven Neumann <sven@gimp.org>
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

#ifndef __GFX_H__
#define __GFX_H__


typedef struct _GfxEngine GfxEngine;

struct _GfxEngine
{
  int            width;
  int            height;
  unsigned char *matrix;

  void (* init)    (GfxEngine     *self,
                    void          *data);
  void (* release) (GfxEngine     *self);
  void (* draw)    (GfxEngine     *self);
};

int  init_gfx          (int         *argc,
                        char       **argv[],
                        int          rows,
                        int          columns,
                        const char  *gfx_engine);
void release_gfx       (void);
void draw_empty_screen (void);
void draw_game_screen  (int          ball_x,
                        int          ball_y,
                        int          lpaddle,
                        int          rpaddle);
int  draw_text_screen  (const char  *text);
int  check_events      (int         *lpaddle,
                        int         *rpaddle);


#endif /*  __GFX_H__  */

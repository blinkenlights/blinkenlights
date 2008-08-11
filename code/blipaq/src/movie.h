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

#ifndef __MOVIE_H__
#define __MOVIE_H__


struct _Movie
{
  char       *filename;
  int         duration;
  MovieFrame *frames;
  MovieFrame *current;

  void         (* Rewind)        (Movie *movie);
  MovieFrame * (* NextFrame)     (Movie *movie, int *duration);
  MovieFrame * (* PreviousFrame) (Movie *movie, int *duration);
  void         (* Release)       (Movie *movie);
};

struct _MovieFrame
{
  int            start;
  int            duration;
  unsigned char  data[18 * 8];

  MovieFrame    *prev;
  MovieFrame    *next;
};

Movie *movie_new (char *filename);


#endif /* __MOVIE_H__ */

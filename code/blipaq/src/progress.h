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

#ifndef __PROGRESS_H__
#define __PROGRESS_H__


#define PROGRESS_SHIFT 10

struct _Progress
{
  IDirectFBSurface *surface;
  IDirectFBSurface *bar;
  IDirectFBSurface *thru;
  int               width;
  int               height;

  void (* SetValue) (Progress *progress,
                     int       value);
  void (* Release)  (Progress *progress);
};


Progress * progress_new (Display          *display,
                         IDirectFBSurface *surface);


#endif /* __PROGRESS_H__ */

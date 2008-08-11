/* blib - Library of useful things to hack the Blinkenlights
 *
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

/* GIF savinging routines stripped out of the GIF saving filter for The GIMP.
 *
 * Modified for blinkentools in 2001 by Sven Neumann  <sven@gimp.org>.
 * Included into blib in 2002 by Sven Neumann  <sven@gimp.org>.
 *
 * GIMP plug-in written by Adam D. Moss  <adam@gimp.org> <adam@foxbox.org>
 *
 * Based around original GIF code by David Koblas.
 *
 * This filter uses code taken from the "giftopnm" and "ppmtogif" programs
 *    which are part of the "netpbm" package.
 *
 *  "The Graphics Interchange Format(c) is the Copyright property of
 *  CompuServe Incorporated.  GIF(sm) is a Service Mark property of
 *  CompuServe Incorporated." 
 */

/* Copyright notice for GIF code from which this plugin was long ago     *
 * derived (David Koblas has granted permission to relicense):           *
 * +-------------------------------------------------------------------+ *
 * | Copyright 1990, 1991, 1993, David Koblas.  (koblas@extra.com)     | *
 * +-------------------------------------------------------------------+ */


#ifndef __GIF_SAVE_H__
#define __GIF_SAVE_H__

#include <blib/gif-types.h>

#ifdef  __cplusplus
extern "C" {
#endif

void GIFEncodeHeader            (FILE           *fp,
                                 int             gif89,
                                 int             Width,
                                 int             Height,
                                 int             Background,
                                 int             BitsPerPixel,
                                 char           *cmap);
void GIFEncodeGraphicControlExt (FILE           *fp,
                                 GIFDisposeType  Disposal,
                                 int             Delay,
                                 int             Animation,
                                 int             Transparent);
void GIFEncodeImageData         (FILE           *fp,
                                 int             Width,
                                 int             Height,
                                 int             BitsPerPixel,
                                 int             offset_x,
                                 int             offset_y,
                                 char           *data);
void GIFEncodeClose             (FILE           *fp);
void GIFEncodeCommentExt        (FILE           *fp,
                                 char           *comment);
void GIFEncodeLoopExt           (FILE           *fp,
                                 int             num_loops);

#ifdef  __cplusplus
}
#endif

#endif /* __GIF_SAVE_H__ */

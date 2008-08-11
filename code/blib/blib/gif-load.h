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

/* GIF loading routines stripped out of the GIF loading filter for The GIMP.
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


#ifndef __GIF_LOAD_H__
#define __GIF_LOAD_H__

#include <blib/gif-types.h>

G_BEGIN_DECLS

int  GIFDecodeHeader            (GIOChannel      *io,
                                 gboolean         skip_magic,
                                 int             *Width,
                                 int             *Height,
                                 int             *Background,
                                 int             *colors,
                                 unsigned char  **cmap);
int  GIFDecodeRecordType        (GIOChannel      *io,
                                 GIFRecordType   *type);
int  GIFDecodeImage             (GIOChannel      *io,
                                 int             *Width,
                                 int             *Height,
                                 int             *offx,
                                 int             *offy, 
                                 int             *colors,
                                 unsigned char  **cmap,
                                 unsigned char   *data);
int  GIFDecodeGraphicControlExt (GIOChannel      *io,
                                 GIFDisposeType  *Disposal,
                                 int             *Delay,
                                 int             *Transparent);
int  GIFDecodeCommentExt        (GIOChannel      *io,
                                 char           **comment);
void GIFDecodeUnknownExt        (GIOChannel      *io);

#ifdef  __cplusplus
}
#endif

#endif /* __GIF_LOAD_H__ */

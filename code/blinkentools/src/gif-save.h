/* GIF savinging routines stripped out of the GIF saving filter for The GIMP.
 *
 * Modified for blinkentools by Sven Neumann  <sven@gimp.org>
 *
 * GIMP plug-in written by Adam D. Moss   <adam@gimp.org> <adam@foxbox.org>
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

/* Copyright notice for code which this plugin was long ago derived from */
/* +-------------------------------------------------------------------+ */
/* | Copyright 1990, 1991, 1993, David Koblas.  (koblas@netcom.com)    | */
/* |   Permission to use, copy, modify, and distribute this software   | */
/* |   and its documentation for any purpose and without fee is hereby | */
/* |   granted, provided that the above copyright notice appear in all | */
/* |   copies and that both that copyright notice and this permission  | */
/* |   notice appear in supporting documentation.  This software is    | */
/* |   provided "as is" without express or implied warranty.           | */
/* +-------------------------------------------------------------------+ */

#ifndef __GIF_SAVE_H__
#define __GIF_SAVE_H__


typedef enum
{
  DISPOSE_UNSPECIFIED,
  DISPOSE_COMBINE,
  DISPOSE_REPLACE
} GIFDisposeType;

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


#endif /* __GIF_SAVE_H__ */

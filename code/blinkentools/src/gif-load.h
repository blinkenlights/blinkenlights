/* GIF loading routines stripped out of the GIF loading filter for The GIMP.
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

#ifndef __GIF_LOAD_H__
#define __GIF_LOAD_H__


typedef enum
{
  DISPOSE_UNSPECIFIED,
  DISPOSE_COMBINE,
  DISPOSE_REPLACE
} GIFDisposeType;

typedef enum
{
  IMAGE,
  GRAPHIC_CONTROL_EXTENSION,
  COMMENT_EXTENSION,
  UNKNOWN_EXTENSION,
  TERMINATOR
} GIFRecordType;

int  GIFDecodeHeader            (FILE            *fd,
                                 int             *Width,
                                 int             *Height,
                                 int             *Background,
                                 int             *colors,
                                 unsigned char  **cmap);
int  GIFDecodeRecordType        (FILE            *fd,
                                 GIFRecordType   *type);
int  GIFDecodeImage             (FILE            *fd,
                                 int             *Width,
                                 int             *Height,
                                 int             *offx,
                                 int             *offy, 
                                 int             *colors,
                                 unsigned char  **cmap,
                                 unsigned char   *data);
int  GIFDecodeGraphicControlExt (FILE            *fd,
                                 GIFDisposeType  *Disposal,
                                 int             *Delay,
                                 int             *Transparent);
int  GIFDecodeCommentExt        (FILE            *fd,
                                 char           **comment);
void GIFDecodeUnknownExt        (FILE            *fd);


#endif /* __GIF_LOAD_H__ */

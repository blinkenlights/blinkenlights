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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gif-load.h"


#define MAX_LZW_BITS     12

#define INTERLACE          0x40
#define LOCALCOLORMAP      0x80
#define BitSet(byte, bit)  (((byte) & (bit)) == (bit))

#define ReadOK(file,buffer,len) (fread(buffer, len, 1, file) != 0)
#define LM_to_uint(a,b)         (((b)<<8)|(a))


static int ReadColorMap (FILE          *fd,
                         int            number,
                         unsigned char *buffer);
static int GetDataBlock (FILE          *fd,
                         unsigned char *buf);
static int GetCode      (FILE          *fd,
                         int            code_size,
                         int            flag);
static int LZWReadByte  (FILE          *fd,
                         int            flag,
                         int            input_code_size);


int
GIFDecodeHeader (FILE           *fd,
                 int            *Width,
                 int            *Height,
                 int            *Background,
                 int            *colors,
                 unsigned char **cmap)
{
  int           AspectRatio;
  unsigned char buf[16];
  char          version[4];

  if (!ReadOK (fd, buf, 6))
    {
      fprintf (stderr, "GIF: error reading magic number\n");
      return 0;
    }

  if (strncmp ((char *) buf, "GIF", 3) != 0)
    {
      fprintf (stderr, "GIF: not a GIF file\n");
      return 0;
    }

  strncpy (version, (char *) buf + 3, 3);
  version[3] = '\0';

  if ((strcmp (version, "87a") != 0) && (strcmp (version, "89a") != 0))
    {
      fprintf (stderr, "GIF: bad version number, not '87a' or '89a'\n");
      return 0;
    }

  if (!ReadOK (fd, buf, 7))
    {
      fprintf (stderr, "GIF: failed to read screen descriptor\n");
      return 0;
    }

  *Width = LM_to_uint (buf[0], buf[1]);
  *Height = LM_to_uint (buf[2], buf[3]);
  *colors = 2 << (buf[4] & 0x07);
  if (Background)
    *Background = buf[5];

  AspectRatio = buf[6];
  if (AspectRatio != 0 && AspectRatio != 49)
    fprintf (stderr, "GIF: warning - non-square pixels\n");

  *cmap = NULL;

  if (BitSet (buf[4], LOCALCOLORMAP))
    {
      unsigned char *colormap;

      colormap = calloc (3 * *colors, 1);

      /* Global Colormap */
      if (!ReadColorMap (fd, *colors, colormap))
	{
	  fprintf (stderr, "GIF: error reading global colormap\n");
          free (colormap);
	  return 0;
	}
      
      *cmap = colormap;
    }

  return 1;
}

int
GIFDecodeRecordType (FILE           *fd,
                     GIFRecordType  *type)
{
  unsigned char c;

  if (!ReadOK (fd, &c, 1))
    {
      fprintf (stderr, "GIF: EOF / read error on image data\n");
      return 0;
    }
  
  switch (c)
    {
    case ',':
      *type = IMAGE;
      return 1;      

    case '!':
      if (!ReadOK (fd, &c, 1))
        {
          fprintf (stderr, "GIF: EOF / read error on extension function code\n");
          return 0;
        }

      switch (c)
        {
        case 0xf9:
          *type = GRAPHIC_CONTROL_EXTENSION;
          break;
        case 0xfe:
          *type = COMMENT_EXTENSION;
          break;
        case 0x01: /* PLAINTEXT_EXTENSION   */
        case 0xff: /* APPLICATION_EXTENSION */
          *type = UNKNOWN_EXTENSION;
          break;
        default:
          return 0;
        }

      return 1;

    case ';':
      *type = TERMINATOR;
      return 1;

    default:
      fprintf (stderr, "GIF: bogus character 0x%02x, ignoring\n", c);
      return GIFDecodeRecordType (fd, type);
    }

  return 0;
}

int
GIFDecodeImage (FILE           *fd,
                int            *Width,
                int            *Height,
                int            *offx,
                int            *offy, 
                int            *colors,
                unsigned char **cmap,
                unsigned char  *data)
{
  unsigned char  buf[16];
  unsigned char  c;
  unsigned char *dest;
  int local_cmap;
  int interlace;
  int v;
  int width, height;
  int xpos = 0, ypos = 0, pass = 0;

  if (!ReadOK (fd, buf, 9))
    {
      fprintf (stderr, "GIF: couldn't read image data\n");
      return 0;
    }

  *colors = 0;
  *cmap   = NULL;

  local_cmap = BitSet (buf[8], LOCALCOLORMAP);
  
  if (local_cmap)
    {
      unsigned char *colormap;

      *colors = 2 << (buf[8] & 0x07);
      colormap = calloc (3 * *colors, 1);

      if (!ReadColorMap (fd, *colors, colormap))
	{
          free (colormap);
	  fprintf (stderr, "GIF: error reading global colormap\n");
          return 0;
	}
      
      *cmap = colormap;
    }

  *Width  = width  = LM_to_uint (buf[4], buf[5]);
  *Height = height = LM_to_uint (buf[6], buf[7]);
  *offx   = LM_to_uint (buf[0], buf[1]);
  *offy   = LM_to_uint (buf[2], buf[3]);

  interlace = BitSet (buf[8], INTERLACE);

  if (!ReadOK (fd, &c, 1))
    {
      fprintf (stderr, "GIF: EOF / read error on image data\n");
      goto error;
    }

  if (LZWReadByte (fd, 1, c) < 0)
    {
      fprintf (stderr, "GIF: error while reading\n");
      goto error;
    }

  while ((v = LZWReadByte (fd, 0, c)) >= 0)
    {
      dest = data + (ypos * width) + xpos;
      *dest = (unsigned char) v;

      xpos++;
      if (xpos == width)
	{
	  xpos = 0;
	  if (interlace)
	    {
	      switch (pass)
		{
		case 0:
		case 1:
		  ypos += 8;
		  break;
		case 2:
		  ypos += 4;
		  break;
		case 3:
		  ypos += 2;
		  break;
		}

	      if (ypos >= height)
		{
		  pass++;
		  switch (pass)
		    {
		    case 1:
		      ypos = 4;
		      break;
		    case 2:
		      ypos = 2;
		      break;
		    case 3:
		      ypos = 1;
		      break;
		    default:
		      goto fini;
		    }
		}
	    }
	  else
	    {
	      ypos++;
	    }
	}
      if (ypos >= height)
	break;
    }

 fini:
  if (LZWReadByte (fd, 0, c) >= 0)
    fprintf (stderr, "GIF: too much input data, ignoring extra...\n");
  
  return 1;

 error:
  if (*cmap)
    free (*cmap);
  *cmap = NULL;
  return 0;
}

int
GIFDecodeGraphicControlExt (FILE           *fd,
                            GIFDisposeType *Disposal,
                            int            *Delay,
                            int            *Transparent)
{
  unsigned char buf[256];
  
  if (GetDataBlock (fd, buf) < 4)
    return 0;

  *Disposal = (buf[0] >> 2) & 0x7;
  *Delay    = LM_to_uint (buf[1], buf[2]);
      
  if ((buf[0] & 0x1) != 0)
    *Transparent = buf[3];
  else
    *Transparent = -1;
  
  while (GetDataBlock (fd, buf) > 0)
    ;

  return 1;
}

int  
GIFDecodeCommentExt (FILE  *fd,
                     char **comment)
{
  char buf[256];
  int  comment_len;
  int  len;
  
  *comment = NULL;
  comment_len = 0;

  while ((len = GetDataBlock (fd, (unsigned char *) buf)) > 0)
    {
      *comment = realloc (*comment, comment_len + len + 1);
      strncpy (*comment + comment_len, buf, len);
      comment_len += len;
    }

  return 1;
}

void
GIFDecodeUnknownExt (FILE  *fd)
{
  char buf[256];

  while (GetDataBlock (fd, buf) > 0)
    ;
}

static int
ReadColorMap (FILE          *fd,
	      int            number,
	      unsigned char *cmap)
{
  int i;
  unsigned char rgb[3];

  for (i = 0; i < number; ++i)
    {
      if (!ReadOK (fd, rgb, sizeof (rgb)))
	{
	  fprintf (stderr, "GIF: bad colormap\n");
	  return 0;
	}

      *cmap++ = rgb[0];
      *cmap++ = rgb[1];
      *cmap++ = rgb[2];
    }

  return 1;
}


int ZeroDataBlock = 0;

static int
GetDataBlock (FILE          *fd,
	      unsigned char *buf)
{
  unsigned char count;

  if (!ReadOK (fd, &count, 1))
    {
      fprintf (stderr, "GIF: error in getting DataBlock size\n");
      return -1;
    }

  ZeroDataBlock = count == 0;

  if ((count != 0) && (!ReadOK (fd, buf, count)))
    {
      fprintf (stderr, "GIF: error in reading DataBlock\n");
      return -1;
    }

  return count;
}

static int
GetCode (FILE *fd,
	 int   code_size,
	 int   flag)
{
  static unsigned char buf[280];
  static int curbit, lastbit, done, last_byte;
  int i, j, ret;
  unsigned char count;

  if (flag)
    {
      curbit = 0;
      lastbit = 0;
      done = 0;
      last_byte = 2;
      return 0;
    }

  if ((curbit + code_size) >= lastbit)
    {
      if (done)
	{
	  if (curbit >= lastbit)
            fprintf (stderr, "GIF: ran off the end of my bits\n");

	  return -1;
	}

      buf[0] = buf[last_byte - 2];
      buf[1] = buf[last_byte - 1];

      if ((count = GetDataBlock (fd, &buf[2])) <= 0)
	done = 1;

      last_byte = 2 + count;
      curbit = (curbit - lastbit) + 16;
      lastbit = (2 + count) * 8;
    }

  ret = 0;
  for (i = curbit, j = 0; j < code_size; ++i, ++j)
    ret |= ((buf[i / 8] & (1 << (i % 8))) != 0) << j;

  curbit += code_size;

  return ret;
}

static int
LZWReadByte (FILE *fd,
	     int   flag,
	     int   input_code_size)
{
  static int fresh = 0;
  int code, incode;
  static int code_size, set_code_size;
  static int max_code, max_code_size;
  static int firstcode, oldcode;
  static int clear_code, end_code;
  static int table[2][(1 << MAX_LZW_BITS)];
  static int stack[(1 << (MAX_LZW_BITS)) * 2], *sp;
  register int i;

  if (flag)
    {
      set_code_size = input_code_size;
      code_size = set_code_size + 1;
      clear_code = 1 << set_code_size;
      end_code = clear_code + 1;
      max_code_size = 2 * clear_code;
      max_code = clear_code + 2;

      GetCode (fd, 0, 1);

      fresh = 1;

      for (i = 0; i < clear_code; ++i)
	{
	  table[0][i] = 0;
	  table[1][i] = i;
	}
      for (; i < (1 << MAX_LZW_BITS); ++i)
	table[0][i] = table[1][0] = 0;

      sp = stack;

      return 0;
    }
  else if (fresh)
    {
      fresh = 0;
      do
	{
	  firstcode = oldcode =
	    GetCode (fd, code_size, 0);
	}
      while (firstcode == clear_code);
      return firstcode;
    }

  if (sp > stack)
    return *--sp;

  while ((code = GetCode (fd, code_size, 0)) >= 0)
    {
      if (code == clear_code)
	{
	  for (i = 0; i < clear_code; ++i)
	    {
	      table[0][i] = 0;
	      table[1][i] = i;
	    }
	  for (; i < (1 << MAX_LZW_BITS); ++i)
	    table[0][i] = table[1][i] = 0;
	  code_size = set_code_size + 1;
	  max_code_size = 2 * clear_code;
	  max_code = clear_code + 2;
	  sp = stack;
	  firstcode = oldcode =
	    GetCode (fd, code_size, 0);
	  return firstcode;
	}
      else if (code == end_code)
	{
	  int count;
	  unsigned char buf[260];

	  if (ZeroDataBlock)
	    return -2;

	  while ((count = GetDataBlock (fd, buf)) > 0)
	    ;

	  if (count != 0)
	    fprintf (stderr, 
                     "GIF: missing EOD in data stream (common occurence)");
	  return -2;
	}

      incode = code;

      if (code >= max_code)
	{
	  *sp++ = firstcode;
	  code = oldcode;
	}

      while (code >= clear_code)
	{
	  *sp++ = table[1][code];
	  if (code == table[0][code])
	    {
	      fprintf (stderr, "GIF: circular table entry BIG ERROR\n");
	      return -2;
	    }
	  code = table[0][code];
	}

      *sp++ = firstcode = table[1][code];

      if ((code = max_code) < (1 << MAX_LZW_BITS))
	{
	  table[0][code] = oldcode;
	  table[1][code] = firstcode;
	  ++max_code;
	  if ((max_code >= max_code_size) &&
	      (max_code_size < (1 << MAX_LZW_BITS)))
	    {
	      max_code_size *= 2;
	      ++code_size;
	    }
	}

      oldcode = incode;

      if (sp > stack)
	return *--sp;
    }
  return code;
}

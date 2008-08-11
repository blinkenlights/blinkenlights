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


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "gif-save.h"


static int GIFNextPixel (void);

/*
 * a code_int must be able to hold 2**BITS values of type int, and also -1
 */
typedef int code_int;
typedef long int count_int;


unsigned char *pixels;

static void Putword (int, FILE *);
static void compress (int, FILE *);
static void output (code_int);
static void cl_block (void);
static void cl_hash (count_int);
static void writeerr (void);
static void char_init (void);
static void char_out (int);
static void flush_char (void);


static long CountDown;

/*
 * Return the next pixel from the image
 */
int
GIFNextPixel (void)
{
  if (CountDown == 0)
    return EOF;

  --CountDown;

  return *pixels++;
}

/* public */

void
GIFEncodeHeader (FILE    *fp,
		 int      gif89,
		 int      Width,
		 int      Height,
		 int      Background,
		 int      BitsPerPixel,
		 char    *cmap)
{
  int B;
  int Resolution;
  int ColorMapSize;
  int i;

  ColorMapSize = 1 << BitsPerPixel;

  Resolution = BitsPerPixel;

  /*
   * Write the Magic header
   */
  fwrite (gif89 ? "GIF89a" : "GIF87a", 1, 6, fp);

  /*
   * Write out the screen width and height
   */
  Putword (Width, fp);
  Putword (Height, fp);

  /*
   * Indicate that there is a global colour map
   */
  B = 0x80;			/* Yes, there is a color map */

  /*
   * OR in the resolution
   */
  B |= (Resolution - 1) << 5;

  /*
   * OR in the Bits per Pixel
   */
  B |= (BitsPerPixel - 1);

  /*
   * Write it out
   */
  fputc (B, fp);

  /*
   * Write out the Background colour
   */
  fputc (Background, fp);

  /*
   * Byte of 0's (future expansion)
   */
  fputc (0, fp);

  /*
   * Write out the Global Colour Map
   */
  for (i = 0; i < 3 * ColorMapSize; i++)
    fputc (cmap[i], fp);
}

void
GIFEncodeGraphicControlExt (FILE           *fp,
			    GIFDisposeType  Disposal,
			    int             Delay,
			    int             Animation,
			    int             Transparent)
{
  /*
   * Write out extension for transparent colour index, if necessary.
   */
  if ( (Transparent >= 0) || (Animation) )
    {
      /* Extension Introducer - fixed. */
      fputc ('!', fp);
      /* Graphic Control Label - fixed. */
      fputc (0xf9, fp);
      /* Block Size - fixed. */
      fputc (4, fp);
      
      /* Packed Fields - XXXdddut (d=disposal, u=userInput, t=transFlag) */
      /*                    s8421                                        */
      fputc ( ((Transparent >= 0) ? 0x01 : 0x00) /* TRANSPARENCY */

	      /* DISPOSAL */
	      | (Animation ? (Disposal << 2) : 0x00),
	      /* 0x03 or 0x01 build frames cumulatively */
	      /* 0x02 clears frame before drawing */
	      /* 0x00 'don't care' */

	      fp);
      
      fputc (Delay & 255, fp);
      fputc ((Delay>>8) & 255, fp);

      fputc (Transparent, fp);
      fputc (0, fp);
    }
}

void
GIFEncodeImageData (FILE    *fp,
		    int      Width,
		    int      Height,
		    int      BitsPerPixel,
		    int      offset_x,
		    int      offset_y,
                    char    *data)
{
  int Resolution;
  int ColorMapSize;
  int InitCodeSize;
 
  ColorMapSize = 1 << BitsPerPixel;

  Resolution = BitsPerPixel;

  /*
   * Calculate number of bits we are expecting
   */
  CountDown = (long) Width * (long) Height;
  pixels = data;

  /*
   * The initial code size
   */
  if (BitsPerPixel <= 1)
    InitCodeSize = 2;
  else
    InitCodeSize = BitsPerPixel;

  /*
   * Write an Image separator
   */
  fputc (',', fp);

  /*
   * Write the Image header
   */
  Putword (offset_x, fp);
  Putword (offset_y, fp);
  Putword (Width, fp);
  Putword (Height, fp);

  /* no interlacing */
  fputc (0x0, fp);

  /*
   * Write out the initial code size
   */
  fputc (InitCodeSize, fp);

  /*
   * Go and actually compress the data
   */
  compress (InitCodeSize + 1, fp);

  /*
   * Write out a Zero-length packet (to end the series)
   */
  fputc (0x0, fp);
}

void
GIFEncodeClose (FILE *fp)
{
  /*
   * Write the GIF file terminator
   */
  fputc (';', fp);

  /*
   * And close the file
   */
  fclose (fp);
}

void
GIFEncodeLoopExt (FILE *fp,
                  int   num_loops)
{
  fputc(0x21,fp);
  fputc(0xff,fp);
  fputc(0x0b,fp);
  fputs("NETSCAPE2.0",fp);
  fputc(0x03,fp);
  fputc(0x01,fp);
  Putword(num_loops,fp);
  fputc(0x00,fp);

  /* NOTE: num_loops==0 means 'loop infinitely' */
}


void 
GIFEncodeCommentExt (FILE *fp, 
                     char *comment)
{
  if (!comment || !*comment)
    return;

  if (strlen(comment)>240)
    {
      fprintf (stderr, 
               "GIF: warning: comment too large - comment block not written.\n");
      return;
    }

  fputc(0x21,fp);
  fputc(0xfe,fp);
  fputc(strlen(comment),fp);
  fputs((const char *)comment,fp);
  fputc(0x00,fp);
}



/*
 * Write out a word to the GIF file
 */
static void
Putword (int   w,
	 FILE *fp)
{
  fputc (w & 0xff, fp);
  fputc ((w / 256) & 0xff, fp);
}


/***************************************************************************
 *
 *  GIFCOMPR.C       - GIF Image compression routines
 *
 *  Lempel-Ziv compression based on 'compress'.  GIF modifications by
 *  David Rowley (mgardi@watdcsu.waterloo.edu)
 *
 ***************************************************************************/

/*
 * General DEFINEs
 */

#define GIF_BITS    12

#define HSIZE  5003		/* 80% occupancy */

#ifdef NO_UCHAR
typedef char char_type;
#else /*NO_UCHAR */
typedef unsigned char char_type;
#endif /*NO_UCHAR */

/*

 * GIF Image compression - modified 'compress'
 *
 * Based on: compress.c - File compression ala IEEE Computer, June 1984.
 *
 * By Authors:  Spencer W. Thomas       (decvax!harpo!utah-cs!utah-gr!thomas)
 *              Jim McKie               (decvax!mcvax!jim)
 *              Steve Davies            (decvax!vax135!petsd!peora!srd)
 *              Ken Turkowski           (decvax!decwrl!turtlevax!ken)
 *              James A. Woods          (decvax!ihnp4!ames!jaw)
 *              Joe Orost               (decvax!vax135!petsd!joe)
 *
 */
#include <ctype.h>

#define ARGVAL() (*++(*argv) || (--argc && *++argv))

static int n_bits;		/* number of bits/code */
static int maxbits = GIF_BITS;	/* user settable max # bits/code */
static code_int maxcode;	/* maximum code, given n_bits */
static code_int maxmaxcode = (code_int) 1 << GIF_BITS;	/* should NEVER generate this code */
#ifdef COMPATIBLE		/* But wrong! */
#define MAXCODE(Mn_bits)        ((code_int) 1 << (Mn_bits) - 1)
#else /*COMPATIBLE */
#define MAXCODE(Mn_bits)        (((code_int) 1 << (Mn_bits)) - 1)
#endif /*COMPATIBLE */

static count_int htab[HSIZE];
static unsigned short codetab[HSIZE];
#define HashTabOf(i)       htab[i]
#define CodeTabOf(i)    codetab[i]

const code_int hsize = HSIZE;	/* the original reason for this being
				   variable was "for dynamic table sizing",
				   but since it was never actually changed
				   I made it const   --Adam. */

/*
 * To save much memory, we overlay the table used by compress() with those
 * used by decompress().  The tab_prefix table is the same size and type
 * as the codetab.  The tab_suffix table needs 2**GIF_BITS characters.  We
 * get this from the beginning of htab.  The output stack uses the rest
 * of htab, and contains characters.  There is plenty of room for any
 * possible stack (stack used to be 8000 characters).
 */

#define tab_prefixof(i) CodeTabOf(i)
#define tab_suffixof(i)        ((char_type*)(htab))[i]
#define de_stack               ((char_type*)&tab_suffixof((code_int)1<<GIF_BITS))

static code_int free_ent = 0;	/* first unused entry */

/*
 * block compression parameters -- after all codes are used up,
 * and compression rate changes, start over.
 */
static int clear_flg = 0;

static int offset;
static long int in_count = 1;	/* length of input */
static long int out_count = 0;	/* # of codes output (for debugging) */

/*
 * compress stdin to stdout
 *
 * Algorithm:  use open addressing double hashing (no chaining) on the
 * prefix code / next character combination.  We do a variant of Knuth's
 * algorithm D (vol. 3, sec. 6.4) along with G. Knott's relatively-prime
 * secondary probe.  Here, the modular division first probe is gives way
 * to a faster exclusive-or manipulation.  Also do block compression with
 * an adaptive reset, whereby the code table is cleared when the compression
 * ratio decreases, but after the table fills.  The variable-length output
 * codes are re-sized at this point, and a special CLEAR code is generated
 * for the decompressor.  Late addition:  construct the table according to
 * file size for noticeable speed improvement on small files.  Please direct
 * questions about this implementation to ames!jaw.
 */

static int g_init_bits;
static FILE *g_outfile;

static int ClearCode;
static int EOFCode;


static unsigned long cur_accum;
static int cur_bits;

static unsigned long masks[] =
{0x0000, 0x0001, 0x0003, 0x0007, 0x000F,
 0x001F, 0x003F, 0x007F, 0x00FF,
 0x01FF, 0x03FF, 0x07FF, 0x0FFF,
 0x1FFF, 0x3FFF, 0x7FFF, 0xFFFF};


static void
compress (int      init_bits,
	  FILE    *outfile)
{
  register long fcode;
  register code_int i /* = 0 */ ;
  register int c;
  register code_int ent;
  register code_int disp;
  register code_int hsize_reg;
  register int hshift;


  /*
   * Set up the globals:  g_init_bits - initial number of bits
   *                      g_outfile   - pointer to output file
   */
  g_init_bits = init_bits;
  g_outfile = outfile;

  cur_bits = 0;
  cur_accum = 0;

  /*
   * Set up the necessary values
   */
  offset = 0;
  out_count = 0;
  clear_flg = 0;
  in_count = 1;

  ClearCode = (1 << (init_bits - 1));
  EOFCode = ClearCode + 1;
  free_ent = ClearCode + 2;


  /* Had some problems here... should be okay now.  --Adam */
  n_bits = g_init_bits;
  maxcode = MAXCODE (n_bits);

  char_init ();

  ent = GIFNextPixel ();

  hshift = 0;
  for (fcode = (long) hsize; fcode < 65536L; fcode *= 2L)
    ++hshift;
  hshift = 8 - hshift;		/* set hash code range bound */

  hsize_reg = hsize;
  cl_hash ((count_int) hsize_reg);	/* clear hash table */

  output ((code_int) ClearCode);

  while ((c = GIFNextPixel()) != EOF)
    {
      ++in_count;

      fcode = (long) (((long) c << maxbits) + ent);
      i = (((code_int) c << hshift) ^ ent);	/* xor hashing */

      if (HashTabOf (i) == fcode)
	{
	  ent = CodeTabOf (i);
	  continue;
	}
      else if ((long) HashTabOf (i) < 0)	/* empty slot */
	goto nomatch;
      disp = hsize_reg - i;	/* secondary hash (after G. Knott) */
      if (i == 0)
	disp = 1;
    probe:
      if ((i -= disp) < 0)
	i += hsize_reg;

      if (HashTabOf (i) == fcode)
	{
	  ent = CodeTabOf (i);
	  continue;
	}
      if ((long) HashTabOf (i) > 0)
	goto probe;
    nomatch:
      output ((code_int) ent);
      ++out_count;
      ent = c;

      if (free_ent < maxmaxcode)
	{
	  CodeTabOf (i) = free_ent++;	/* code -> hashtable */
	  HashTabOf (i) = fcode;
	}
      else
	cl_block ();
    }
  /*
   * Put out the final code.
   */
  output ((code_int) ent);
  ++out_count;
  output ((code_int) EOFCode);
}

/*****************************************************************
 * TAG( output )
 *
 * Output the given code.
 * Inputs:
 *      code:   A n_bits-bit integer.  If == -1, then EOF.  This assumes
 *              that n_bits =< (long)wordsize - 1.
 * Outputs:
 *      Outputs code to the file.
 * Assumptions:
 *      Chars are 8 bits long.
 * Algorithm:
 *      Maintain a GIF_BITS character long buffer (so that 8 codes will
 * fit in it exactly).  Use the VAX insv instruction to insert each
 * code in turn.  When the buffer fills up empty it and start over.
 */

static void
output (code_int code)
{
  cur_accum &= masks[cur_bits];

  if (cur_bits > 0)
    cur_accum |= ((long) code << cur_bits);
  else
    cur_accum = code;

  cur_bits += n_bits;

  while (cur_bits >= 8)
    {
      char_out ((unsigned int) (cur_accum & 0xff));
      cur_accum >>= 8;
      cur_bits -= 8;
    }

  /*
   * If the next entry is going to be too big for the code size,
   * then increase it, if possible.
   */
  if (free_ent > maxcode || clear_flg)
    {
      if (clear_flg)
	{

	  maxcode = MAXCODE (n_bits = g_init_bits);
	  clear_flg = 0;

	}
      else
	{

	  ++n_bits;
	  if (n_bits == maxbits)
	    maxcode = maxmaxcode;
	  else
	    maxcode = MAXCODE (n_bits);
	}
    }

  if (code == EOFCode)
    {
      /*
       * At EOF, write the rest of the buffer.
       */
      while (cur_bits > 0)
	{
	  char_out ((unsigned int) (cur_accum & 0xff));
	  cur_accum >>= 8;
	  cur_bits -= 8;
	}

      flush_char ();

      fflush (g_outfile);

      if (ferror (g_outfile))
	writeerr ();
    }
}

/*
 * Clear out the hash table
 */
static void
cl_block ()			/* table clear for block compress */
{

  cl_hash ((count_int) hsize);
  free_ent = ClearCode + 2;
  clear_flg = 1;

  output ((code_int) ClearCode);
}

static void
cl_hash (register count_int hsize)			/* reset code table */
{

  register count_int *htab_p = htab + hsize;

  register long i;
  register long m1 = -1;

  i = hsize - 16;
  do
    {				/* might use Sys V memset(3) here */
      *(htab_p - 16) = m1;
      *(htab_p - 15) = m1;
      *(htab_p - 14) = m1;
      *(htab_p - 13) = m1;
      *(htab_p - 12) = m1;
      *(htab_p - 11) = m1;
      *(htab_p - 10) = m1;
      *(htab_p - 9) = m1;
      *(htab_p - 8) = m1;
      *(htab_p - 7) = m1;
      *(htab_p - 6) = m1;
      *(htab_p - 5) = m1;
      *(htab_p - 4) = m1;
      *(htab_p - 3) = m1;
      *(htab_p - 2) = m1;
      *(htab_p - 1) = m1;
      htab_p -= 16;
    }
  while ((i -= 16) >= 0);

  for (i += 16; i > 0; --i)
    *--htab_p = m1;
}

static void
writeerr ()
{
  fprintf (stderr, "GIF: error writing output file\n");
  return;
}

/******************************************************************************
 *
 * GIF Specific routines
 *
 ******************************************************************************/

/*
 * Number of characters so far in this 'packet'
 */
static int a_count;

/*
 * Set up the 'byte output' routine
 */
static void
char_init ()
{
  a_count = 0;
}

/*
 * Define the storage for the packet accumulator
 */
static char accum[256];

/*
 * Add a character to the end of the current packet, and if it is 254
 * characters, flush the packet to disk.
 */
static void
char_out (int c)
{
  accum[a_count++] = c;
  if (a_count >= 254)
    flush_char ();
}

/*
 * Flush the packet to disk, and reset the accumulator
 */
static void
flush_char (void)
{
  if (a_count > 0)
    {
      fputc (a_count, g_outfile);
      fwrite (accum, 1, a_count, g_outfile);
      a_count = 0;
    }
}

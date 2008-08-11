/* gif2blm.c
 * Converts GIF animations to BlinkenLights movies.
 *
 * Copyright (C) 2001  Sven Neumann <sven@gimp.org>
 * Enhanced by Tino Schwarze <tino.schwarze@informatik.tu-chemnitz.de>
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

#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gif-load.h"
#include "hdl.h"


#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

#define DEFAULT_DELAY 10   /* 1/100 sec */


int
main (int   argc,
      char *argv[])
{
  FILE           *gif;
  GIFRecordType   type;
  unsigned char  *image;
  unsigned char  *frame;
  unsigned char  *cmap;
  char           *data;
  char           *filename = NULL;
  int             i;
  int             help;
  int             version;
  int             width, height, colors;
  int             delay, transparent;
  int             invert = FALSE;
  int             hdl_mode = FALSE;
  GIFDisposeType  disposal;
  HdlInfo hdls[LAST_IMAGE] =
  {
    hdl_plain,
    hdl_small,
    hdl_medium,
    hdl_large,
    hdl_huge
  };
  HdlInfo        *hdl = NULL;
  int             dx, dy;
  int             offx, offy;
  int             bl_width, bl_height;

  disposal    = DISPOSE_COMBINE;
  transparent = -1;
  delay       = DEFAULT_DELAY;

  for (i = 1, help = FALSE, version = FALSE; 
       i < argc && !help && !version; 
       i++)
    {
      if (strncmp (argv[i], "--", 2) == 0)
        {
          if (strcmp (argv[i], "--invert") == 0)
            invert = TRUE;
          else if (strcmp (argv[i], "--hdl") == 0)
            hdl_mode = TRUE;
          else if (strcmp (argv[i], "--version") == 0)
            version = TRUE;
          else
            help = TRUE;
        }
      else if (!filename)
        filename = argv[i];
    }

  if (version)
    {
      fprintf (stderr, "gif2blm version %s\n", VERSION);
      return 0;
    }

  if (help || !filename)
    {
      fprintf (stderr, "\ngif2blm creates Blinkenlights Movies from animated GIFs.\n\n"); 
      fprintf (stderr, "Usage: gif2blm [options] <filename>\n\n");
      fprintf (stderr, "Options:\n");
      fprintf (stderr, "   --invert       Invert output.\n");
      fprintf (stderr, "   --hdl          Use HDL images as input.\n");
      fprintf (stderr, "   --help         Output usage information.\n");
      fprintf (stderr, "   --version      Output version information.\n");
      fprintf (stderr, "\nCheck http://www.blinkenlights.de/ for more information.\n\n");

      return (help ? 0 : -1);
    }

  if (strcmp (filename, "-") == 0)
    gif = stdin;
  else
    gif = fopen (filename, "rb");

  if (!gif)
    {
      fprintf (stderr, "Error opening GIF file '%s'\n", filename);
      return -1;
    }

  if (!GIFDecodeHeader (gif, &width, &height, NULL, &colors, &cmap))
    return -1;

  if (hdl_mode)
    {
      for (i = 1; i < LAST_IMAGE; i++)
        {
          if ((width == hdls[i].width) && (height == hdls[i].height))
            hdl = &hdls[i];
        }

      if (hdl == NULL)
        {
          fprintf (stderr, 
                   "Error: The hdl mode only understand pictures created by blm2gif.\n");
          fprintf (stderr, "The following resolutions are supported:\n");
          for (i = 1; i < LAST_IMAGE; i++)
            fprintf (stderr, "%d x %d\n", hdls[i].width, hdls[i].height);

          return -1;
        }

      bl_width  = BLINKEN_WIDTH;
      bl_height = BLINKEN_HEIGHT;
      dx = hdl->dx; 
      dy = hdl->dy; 
      offx = hdl->offx; 
      offy = hdl->offy;
    }
  else
    {
      bl_width  = width;
      bl_height = height;
      dx = 1;
      dy = 1;
      offx = 0;
      offy = 0;
    }

  printf ("# BlinkenLights Movie %dx%d\n", bl_width, bl_height);
  
  data  = malloc (bl_width * bl_height);
  memset (data, (invert ? '1' : '0'), bl_width * bl_height);

  image = calloc (width * height, 1);
  frame = malloc (width * height);

  while (GIFDecodeRecordType (gif, &type))
    {
      switch (type)
        {
        case GRAPHIC_CONTROL_EXTENSION:
          if (!GIFDecodeGraphicControlExt (gif, 
                                           &disposal, &delay, &transparent))
            return -1;
          break;
          
        case IMAGE:
          {
            unsigned char *lcmap, *effective_cmap;
            unsigned char *src, *win_src;
            unsigned char *img_dest;
            char          *dest;
            unsigned char  value;
            unsigned char *cme;
            int            x, y, xw, yw;
            int            frame_width, frame_height;
            int            left, top;
            int            lcolors;
            int            trans;
            int            sum, count;

            if (!GIFDecodeImage (gif, 
                                 &frame_width, &frame_height, &left, &top, 
                                 &lcolors, &lcmap, frame))
              return -1;
            
            if (lcmap)
                effective_cmap = lcmap;
            else
                effective_cmap = cmap;

            src  = frame;
            img_dest = image + top*width + left;

            /* merge last frame with new (possibly combined) frame */
            for (y = 0; y < frame_height; y++)
            {
                for (x = 0; x < frame_width; x++, src++)
                {
                    value = *src;

                    trans = (transparent > -1) && (value == transparent);

                    switch (disposal)
                      {
                      case DISPOSE_COMBINE:
                        if (!trans)
                          img_dest[x] = value;
                        break;
                      default:
                        img_dest[x] = value;
                        break;
                      }
                }
                img_dest += width;
            }

            src = image + offy*width + offx;
            dest = data;

            /* now figure out the black and white patterns */
            for (y = 0; y < bl_height; y++)
              {
                for (x = 0; x < bl_width; x++, dest++)
                  {
                      sum = count = 0;

                      win_src = src + x*dx;

                      /* sum up intensities of whole window */
                      for (yw = 0; yw < dy; yw++)
                      {
                          for (xw = 0; xw < dx; xw++)
                          {
                              /* only count non-transparent pixels */
                              if ((transparent < 0) || (win_src[xw] != transparent))
                              {
                                  /* calc address of colormap entry */
                                  cme = effective_cmap + (int)win_src[xw]*3;
                                  sum += cme[0] * 30 + cme[1] * 59 + cme[2] * 11;
                                  count++;
                              }

                          }
                          win_src += width;
                      }

                      /* we consider all transparent as black */
                      if (count == 0)
                          *dest = (invert ? '1' : '0' );
                      else
                          *dest = ((sum/count/100) < 50 ? (invert ? '1' : '0') : (invert ? '0' : '1'));

                  }

                  src += dy * width;
              }

            printf ("\n@%d\n", delay * 10);

            dest = data;
            for (y = 0; y < bl_height; y++)
              {
                for (x = 0; x < bl_width; x++, dest++)
                  putchar (*dest);
                putchar ('\n');
              }

            if (lcmap)
              free (lcmap);
          }
          break;
          
        case COMMENT_EXTENSION:
          {
            char *comment;
            int   len;

            if (!GIFDecodeCommentExt (gif, &comment))
              return -1;

            if (comment && (len = strlen (comment)) > 0)
              {
                putchar ('#');
                for (i = 0; i < len; i++)
                  {
                    putchar (comment[i]);
                    if (comment[i] == '\n' && i+1 < len)
                      putchar ('#');
                  }
                free (comment);
                putchar ('\n');
              }
          }
          break;

        case UNKNOWN_EXTENSION:
          GIFDecodeUnknownExt (gif);
          break;
          
        case TERMINATOR:
          fclose (gif);
          return 0;
        }
    }

  fprintf (stderr, "GIFDecodeRecordType() failed\n");
  return -1;
}

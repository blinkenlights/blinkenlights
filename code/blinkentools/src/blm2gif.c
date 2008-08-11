/* blm2gif.c
 * Creates GIF animations from BlinkenLights movies.
 *
 * Copyright (C) 2001  Sven Neumann <sven@gimp.org>
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
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#include "blutils.h"
#include "gif-save.h"

#include "hdl.h"


#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif


static void
write_image (FILE          *gif,
             unsigned char *data,
             int            pitch,
             int            left,
             int            top,
             int            width,
             int            height,
             int            delay,
             HdlInfo       *hdl)
{
  static char *image = NULL;
  char *src, *dest;
  int   first = FALSE;
  int   i, x, y;

  /* if the frame is empty, create a dummy frame to keep the timing intact */
  if (width <= 0 || height <= 0)
    {
      left = top = 0;
      width = height = 1;
    }

  GIFEncodeGraphicControlExt (gif, DISPOSE_COMBINE, delay, TRUE, hdl->trans); 

  if (!image)
    {
      image = malloc (hdl->width * hdl->height);
      memcpy (image, hdl->off, hdl->width * hdl->height);
      dest = image + hdl->width * (hdl->offy + top * hdl->dy);
      first = TRUE;
    }
  else
    {
      dest = image;
    }

  data += top * pitch;
        
  for (y = top; y < top + height; y++)
    {
      for (i = 0; i < hdl->dy; i++)
        {          
          if (first)
            dest += hdl->offx + left * hdl->dy;

          for (x = left; x < left + width; x++)
            {
              if (hdl->trans != -1 && (data[x] & 0x02) && !first)
                {
                  memset (dest, hdl->trans, hdl->dx);
                }
              else
                {
                  src = (data[x] & 0x1) ? hdl->on : hdl->off;
                  src += hdl->width * (hdl->offy + y * hdl->dy + i);
                  src += hdl->offx + x * hdl->dx;
              
                  memcpy (dest, src, hdl->dx);
                }

              dest += hdl->dx;
            }

          if (first)
            dest += hdl->width - (hdl->offx + x * hdl->dx);
        }
      
      data += pitch;
    }
  
  if (first)
    GIFEncodeImageData (gif, hdl->width, hdl->height, hdl->cbits, 0, 0, image);
  else
    GIFEncodeImageData (gif, 
                        width * hdl->dx, height * hdl->dy, hdl->cbits,
                        hdl->offx + left * hdl->dx, hdl->offy + top * hdl->dy,
                        image);
}

int
main (int   argc,
      char *argv[])
{
  unsigned char *data = NULL;
  FILE *blm      = NULL;
  char *filename = NULL;
  char *comment  = NULL;
  char  buf[4096];
  int   comment_len = 0;
  int   width, height;
  int   x1, x2, y1, y2;
  int   lc;
  int   line;
  int   len, i;
  int   duration;
  int   help;
  int   version;
  int   loop = -1;

  HdlInfo hdls[LAST_IMAGE] = 
  {
    hdl_plain,
    hdl_small,
    hdl_medium,
    hdl_large,
    hdl_huge
  };
  ImageType  type = IMAGE_PLAIN;
  HdlInfo   *hdl; 


  for (i = 1, help = FALSE, version = FALSE; 
       i < argc && !help && !version; 
       i++)
    {
      if (strncmp (argv[i], "--", 2) == 0)
        {
          if (strcmp (argv[i], "--hdl-small") == 0)
            type = IMAGE_HDL_SMALL;
          else if (strcmp (argv[i], "--hdl") == 0 || 
                   strcmp (argv[i], "--hdl-medium") == 0)
            type = IMAGE_HDL_MEDIUM;
          else if (strcmp (argv[i], "--hdl-large") == 0)
            type = IMAGE_HDL_LARGE;
          else if (strcmp (argv[i], "--hdl-huge") == 0)
            type = IMAGE_HDL_HUGE;
          else if (strcmp (argv[i], "--loop") == 0)
            {
              loop = 0;
              if (i + 1 < argc && sscanf (argv[i+1], "%d", &loop) == 1)
                i++;
            }
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
      fprintf (stderr, "blm2gif version %s\n", VERSION);
      return 0;
    }

  if (help || !filename)
    {
      fprintf (stderr, "\nblm2gif creates animated GIFs from Blinkenlights Movies.\n\n");
      fprintf (stderr, "Usage: blm2gif [options] <filename>\n\n");
      fprintf (stderr, "Options:\n");
      fprintf (stderr, "   --hdl-small    Haus des Lehrers (%dx%d).\n", 
               hdls[IMAGE_HDL_SMALL].width, hdls[IMAGE_HDL_SMALL].height);
      fprintf (stderr, "   --hdl-medium   Haus des Lehrers (%dx%d).\n", 
               hdls[IMAGE_HDL_MEDIUM].width, hdls[IMAGE_HDL_MEDIUM].height);
      fprintf (stderr, "   --hdl-large    Haus des Lehrers (%dx%d).\n", 
               hdls[IMAGE_HDL_LARGE].width, hdls[IMAGE_HDL_LARGE].height);
      fprintf (stderr, "   --hdl-huge     Haus des Lehrers (%dx%d).\n", 
               hdls[IMAGE_HDL_HUGE].width, hdls[IMAGE_HDL_HUGE].height);
      fprintf (stderr, "   --loop [n]     Loop (infinitely or as specified).\n");
      fprintf (stderr, "   --help         Output usage information.\n");
      fprintf (stderr, "   --version      Output version information.\n");
      fprintf (stderr, "\nCheck http://www.blinkenlights.de/ for more information.\n\n");

      return (help ? 0 : -1);
    }

  if (isatty (1))
    {
      fprintf (stderr, "Not writing to <stdout>: it's a terminal\n");
      return -1;
    }

  if (strcmp (filename, "-") == 0)
    blm = stdin;
  else
    blm = fopen (filename, "r");

  if (!blm)
    {
      fprintf (stderr, "Can't open '%s': %s\n", filename, strerror (errno));
      return -1;
    }
  
  lc = 1;
  if (!bl_fgets (buf, sizeof (buf), blm) && lc++)
    goto blerror;

  if (buf[0] != '#')
    goto blerror;

  i = 1;
  while (isspace (buf[i]))
    i++;

  if (strncasecmp (buf + i, "BlinkenLights Movie", 19) != 0)
    goto blerror;

  if (sscanf (buf + i + 19, "%dx%d", &width, &height) != 2)
    {
      fprintf (stderr, 
               "Blinkenlights files should declare width and height in the first line.\n"
               "This one doesn't but I'll assume %d x %d and try to continue.\n", 
               BLINKEN_WIDTH, BLINKEN_HEIGHT);

      width  = BLINKEN_WIDTH;
      height = BLINKEN_HEIGHT;
    }

  if (type != IMAGE_PLAIN && 
      ((width != BLINKEN_WIDTH) || (height != BLINKEN_HEIGHT)))
    {
      fprintf (stderr, "Sorry, HDL mode is only available for size %dx%d.\n",
               BLINKEN_WIDTH, BLINKEN_HEIGHT);
      return -1;
    }

  hdl = &hdls[type];

  if (type == IMAGE_PLAIN)
    {
      hdl->width  = width;
      hdl->height = height;
      hdl->on = malloc (width * height);
      memset (hdl->on, 1, width * height);
      hdl->off = malloc (width * height);
      memset (hdl->off, 0, width * height);
    }

  GIFEncodeHeader (stdout, TRUE, 
                   hdl->width, hdl->height, 0, hdl->cbits, hdl->colors);
  
  if (loop >= 0)
    GIFEncodeLoopExt (stdout, loop);

  data = calloc (width * height, sizeof (unsigned char));  

  line = -1;
  x1 = y1 = 0;
  x2 = width;
  y2 = height;

  while (bl_fgets (buf, sizeof (buf), blm) && lc++)
    {
      len = strlen (buf);

      if (len == 0)
        continue;
      
      if (buf[0] == '#' && comment_len < 240)
        {
          len--;
          if (comment_len + len > 240)
            len = 240 - comment_len;
          comment = realloc (comment, comment_len + len + 1);
          strncpy (comment + comment_len, buf + 1, len);
          comment_len += len;
          continue;
        }
      
      break;
    }

  if (feof (blm))
    goto blerror;
     
  if (comment)
    {
      GIFEncodeCommentExt (stdout, comment);
      free (comment);
    }

  do 
    {
      len = strlen (buf);

      if (len == 0)
        continue;

      if (line == -1)
        {
          if (buf[0] == '@')
            {
              if (sscanf (buf+1, "%d", &duration) == 1 && duration > 0)
                line = 0;
            }
        }
      else
        {
          /* special case last line */
          if (feof (blm))
            len++;

          if (buf[0] == '@' || len - 1 < width)
            {
              fprintf (stderr, "Invalid frame, skipping (line %d).\n", lc);
              line = -1;
            }
          else
            {
              unsigned char *dest = data + width * line;
              
              for (i = 0; i < width; i++, dest++)
                {
                  unsigned char pixel = (buf[i] == '0' ? 0x0 : 0x1);

                  if ((*dest & 0x01) == pixel)
                    {
                      *dest |= 0x02;
                    }
                  else
                    {
                      if (x1 > i)        x1 = i;
                      if (x2 < i + 1)    x2 = i + 1;
                      if (y1 > line)     y1 = line;
                      if (y2 < line + 1) y2 = line + 1;

                      *dest = pixel;
                    }
                }

              if (++line == height)
                {
                  write_image (stdout, data, width, 
                               x1, y1, x2 - x1, y2 - y1, duration / 10, hdl);
                  
                  x1 = width;
                  y1 = height;
                  x2 = y2 = 0;
                  line = -1;
                }
            }            
        }
    }
  while (bl_fgets (buf, sizeof (buf), blm) && lc++);

  free (data);
  fclose (blm);
  GIFEncodeClose (stdout); 

  return 0;

 blerror:
  fprintf (stderr, "Error parsing BlinkenLights movie '%s' (line %d).\n", 
           filename, lc);
  if (data)
    free (data);
  fclose (blm);
  fclose (stdout);
  return -1;  
}

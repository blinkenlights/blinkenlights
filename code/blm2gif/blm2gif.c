/* blm2gif.c
 * Creates GIF animations from BlinkenLights movies.
 *
 * Copyright (C) 2001  Sven Neumann <sven@gimp.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>
#include <unistd.h>

#include "gif.h"
#include "hdl.h"
#include "hdl-small.h"
#include "hdl-medium.h"
#include "hdl-large.h"


/* these are only defaults */
#define WIDTH  18
#define HEIGHT  8


#ifndef FALSE
#define FALSE 0
#define TRUE (!FALSE)
#endif

static char * bl_fgets     (char    *s, 
                            int      size, 
                            FILE    *stream);
static void   write_image  (FILE    *gif,
                            char    *data,
                            int      pitch,
                            int      top,
                            int      left,
                            int      width,
                            int      height,
                            int      delay,
                            HdlInfo *hdl);

int
main (int   argc,
      char *argv[])
{
  FILE *blm      = NULL;
  char *filename = NULL;
  char *data     = NULL;
  char *comment  = NULL;
  char  buf[1024];
  int   comment_len = 0;
  int   width, height;
  int   x1, x2, y1, y2;
  int   lc;
  int   line;
  int   len, i;
  int   duration;
  int   help;
  int   loop = -1;
  HdlInfo hdls[4] = 
  {
    hdl_plain,
    hdl_small,
    hdl_medium,
    hdl_large
  };
  ImageType  type = IMAGE_PLAIN;
  HdlInfo   *hdl; 


  for (i = 1, help = 0; i < argc && !help; i++)
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
          else if (strcmp (argv[i], "--loop") == 0)
            {
              loop = 0;
              if (i + 1 < argc && sscanf (argv[i+1], "%d", &loop) == 1)
                i++;
            }
          else
            help = 1;
        }
      else if (!filename)
        filename = argv[i];
    }

  if (help || !filename)
    {
      fprintf (stderr, "blm2gif creates animated GIFs from Blinkenlights Movies.\n\n"); 
      fprintf (stderr, "Usage: blm2gif [options] <filename>\n\n");
      fprintf (stderr, "Options:\n");
      fprintf (stderr, "\t--hdl-small\tHaus des Lehrers (%dx%d)\n", 
               hdls[IMAGE_HDL_SMALL].width, hdls[IMAGE_HDL_SMALL].height);
      fprintf (stderr, "\t--hdl-medium\tHaus des Lehrers (%dx%d)\n", 
               hdls[IMAGE_HDL_MEDIUM].width, hdls[IMAGE_HDL_MEDIUM].height);
      fprintf (stderr, "\t--hdl-large\tHaus des Lehrers (%dx%d)\n", 
               hdls[IMAGE_HDL_LARGE].width, hdls[IMAGE_HDL_LARGE].height);
      fprintf (stderr, "\t--loop [n]\tLoop (infinitely or as specified)\n");
      return -1;
    }

  if (isatty (1))
    {
      fprintf (stderr, "Not writing to <stdout>: it's a terminal\n");
      return -1;
    }

  if (strcmp (filename, "-") == 0)
    blm = stdin;
  else
    blm = fopen (filename, "rb");

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
               WIDTH, HEIGHT);

      width  = WIDTH;
      height = HEIGHT;
    }

  if (type != IMAGE_PLAIN && ((width != WIDTH) || (height != HEIGHT)))
    {
      fprintf (stderr, "Sorry, HDL mode is only available for size %dx%d.\n",
               WIDTH, HEIGHT);
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

  data = calloc (width * height, sizeof (char));  

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
          if (buf[0] == '@' || (len - 1 < width && !feof (blm)))
            {
              fprintf (stderr, "Invalid frame, skipping (line %d).\n", lc);
              line = -1;
            }
          else
            {
              for (i = 0; i < width; i++)
                {
                  char pixel = (buf[i] == '1' ? 1 : 0);
                  if (data[(width * line + i)] != pixel)
                    {
                      if (x1 > i)        x1 = i;
                      if (x2 < i + 1)    x2 = i + 1;
                      if (y1 > line)     y1 = line;
                      if (y2 < line + 1) y2 = line + 1;
                    }
                    
                  data[width * line + i] = pixel;
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

static void
write_image (FILE    *gif,
             char    *data,
             int      pitch,
             int      left,
             int      top,
             int      width,
             int      height,
             int      delay,
             HdlInfo *hdl)
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

  GIFEncodeGraphicControlExt (gif, DISPOSE_COMBINE, delay, 2, -1); 

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
              src = data[x] ? hdl->on : hdl->off;
              src += hdl->width * (hdl->offy + y * hdl->dy + i);
              src += hdl->offx + x * hdl->dx;
              
              memcpy (dest, src, hdl->dx);
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

static char *
bl_fgets (char *s, 
          int   size, 
          FILE *stream)
{
  int i = 0;
  int c = 0;

  if (!s || size < 2)
    return NULL;

  while (i < size - 1)
    {
      c = fgetc (stream);
      if (c == EOF || c == '\r')
        break;
      s[i++] = (char) c;
      if (c == '\n')
        break;
    }

  if (c == '\r')
    {
      c = fgetc (stream);
      if (c != '\n' && c != EOF)
        ungetc (c, stream);
      s[i++] = '\n';
    }
 
  if (i)
    s[i++] = '\0';

  return i > 0 ? s : NULL;
}

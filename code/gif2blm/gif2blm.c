/* gif2blm.c
 * Converts GIF animations to BlinkenLights movies.
 *
 * Copyright (C) 2001  Sven Neumann <sven@gimp.org>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "gif.h"


#define DEFAULT_DELAY 10   /* 1/100 sec */

static int invert = 0;

static char
calc_value (unsigned char *color)
{
  unsigned int intensity;

  intensity = color[0] * 30 + color[1] * 59 + color[2] * 11;

  return (intensity < 50 ? (invert ? '1' : '0') : (invert ? '0' : '1')); 
}

int
main (int   argc,
      char *argv[])
{
  FILE          *gif;
  GIFRecordType  type;
  unsigned char *image;
  unsigned char *cmap;
  char          *data;
  char          *filename = NULL;
  int            i;
  int            help;
  int            width, height, colors;
  int            disposal, delay, transparent;

  disposal    = DISPOSE_COMBINE;
  transparent = 0;
  delay       = DEFAULT_DELAY;

  for (i = 1, help = 0; i < argc && !help; i++)
    {
      if (strncmp (argv[i], "--", 2) == 0)
        {
          if (strcmp (argv[i], "--invert") == 0)
            invert = 1;
          else
            help = 1;
        }
      else if (!filename)
        filename = argv[i];
    }

  if (help || !filename)
    {
      fprintf (stderr, "gif2blm creates Blinkenlights Movies from animated GIFs.\n\n"); 
      fprintf (stderr, "Usage: gif2blm [options] <filename>\n\n");
      fprintf (stderr, "Options:\n");
      fprintf (stderr, "\t--invert\n");
      return -1;
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

  printf ("# BlinkenLights Movie %dx%d\n", width, height);
  
  data  = malloc (width * height);
  memset (data, (invert ? '1' : '0'), width * height);

  image = malloc (width * height);

  while (GIFDecodeRecordType (gif, &type))
    {
      switch (type)
        {
        case IMAGE:
          {
            unsigned char *lcmap;
            unsigned char *src;
            char          *dest;
            char           value;
            int            x, y;
            int            img_width, img_height;
            int            left, top;
            int            lcolors;
            int            trans;

            if (!GIFDecodeImage (gif, 
                                 &img_width, &img_height, &left, &top, 
                                 &lcolors, &lcmap, image))
              return -1;
            
            src  = image;
            dest = data + top * width + left;

            for (y = 0; y < img_height; y++)
              {
                for (x = 0; x < img_width; x++, src++)
                  {
                    trans = (*src == transparent);
                    value = (lcmap ? 
                             calc_value (lcmap + *src * 3) : 
                             calc_value (cmap  + *src * 3));
 
                    switch (disposal)
                      {
                      case DISPOSE_COMBINE:
                        if (!trans)
                          dest[x] = value;
                        break;
                      default:
                        dest[x] = trans ? 0 : value;
                        break;
                      }
                  }
                dest += width;
              }

            printf ("\n@%d\n", delay * 10);

            dest = data;
            for (y = 0; y < height; y++)
              {
                for (x = 0; x < width; x++, dest++)
                  putchar (*dest);
                putchar ('\n');
              }

            if (lcmap)
              free (lcmap);
          }
          break;
          
        case GRAPHIC_CONTROL_EXTENSION:
          if (!GIFDecodeGraphicControlExt (gif, 
                                           &disposal, &delay, &transparent))
            return -1;
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

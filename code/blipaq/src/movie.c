/* 
 * Copyright (C) 2002  Sven Neumann <sven@gimp.org>
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "blipaqtypes.h"
#include "movie.h"


static MovieFrame * movie_next_frame (Movie *movie, int *duration);
static MovieFrame * movie_prev_frame (Movie *movie, int *duration);
static void         movie_rewind     (Movie *movie);
static void         movie_release    (Movie *movie);
static int          movie_load       (Movie *movie);

static char * bl_fgets (char *s, 
                        int   size, 
                        FILE *stream);


Movie *
movie_new (char *filename)
{
  Movie *movie;

  if (!filename)
    return NULL;

  movie = calloc (sizeof (Movie), 1);

  movie->Rewind        = movie_rewind;
  movie->NextFrame     = movie_next_frame;
  movie->PreviousFrame = movie_prev_frame;
  movie->Release       = movie_release;

  movie->filename = strdup (filename);

  if (movie_load (movie) != 0)
    {
      movie->Release (movie);
      return NULL;
    }

  return movie;
}

static void
movie_rewind (Movie *movie)
{
  movie->current = NULL;
}

static MovieFrame *
movie_next_frame (Movie *movie,
                  int   *duration)
{
  MovieFrame *frame;

  if (movie->current)
    frame = movie->current->next;
  else
    frame = movie->frames;
  
  if (frame)
    movie->current = frame;

  if (frame && duration)
    *duration = frame->duration;

  return frame;
}

static MovieFrame *
movie_prev_frame (Movie *movie,
                  int   *duration)
{
  MovieFrame *frame;

  if (movie->current)
    frame = movie->current->prev;
  else
    frame = NULL;
  
  if (frame)
    movie->current = frame;

  if (frame && duration)
    *duration = frame->duration;

  return frame;
}

static void
movie_release (Movie *movie)
{
  MovieFrame *frame;

  if (movie->filename)
    free (movie->filename);

  while (movie->frames)
    {
      frame = movie->frames;
      movie->frames = frame->next;
      free (frame);
    }    
    
  free (movie);
}

static int
movie_load (Movie *movie)
{
  FILE       *file;
  MovieFrame *frame = NULL;
  char        buf[1024];
  int         lc;
  int         line;
  int         width, height;
  int         len, i;

  file = fopen (movie->filename, "r");
  if (!file)
    return 1;

  lc = 1;
  if (!bl_fgets (buf, sizeof (buf), file) && lc++)
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
               "This one doesn't but I'll assume 18 x 8 and try to continue.\n");

      width  = 18;
      height = 8;
    }
  if (width != 18 || height != 8)
    {
      fprintf (stderr, 
               "Sorry, I can only handle BlinkenLights Movies of size 18x8\n");
      goto blerror;
    }

  line = -1;

  while (bl_fgets (buf, sizeof (buf), file) && lc++)
    {
      len = strlen (buf);

      if (len == 0 || buf[0] == '#')
        continue;
      
      if (line == -1)
        {
          if (!frame)
            frame = calloc (sizeof (MovieFrame), 1);

          if (buf[0] == '@')
            {
              if (sscanf (buf+1, "%d", &frame->duration) == 1 && 
                  frame->duration > 0)
                line = 0;
            }
        }
      else
        {
          /* special case last line */
          if (feof (file))
            len++;

          if (buf[0] == '@' || len - 1 < 18)
            {
              fprintf (stderr, "Invalid frame, skipping (line %d).\n", lc);
              line = -1;
            }
          else
            {
              unsigned char *dest = frame->data + 18 * line;
              
              for (i = 0; i < 18; i++, dest++)
                *dest = (buf[i] == '0' ? 0x0 : 0x1);

              if (++line == 8)
                {
                  if (movie->current)
                    {
                      movie->current->next = frame;
                      frame->prev = movie->current;
                    }
                  else
                    {
                      movie->frames = frame;
                    }
                  
                  frame->start = movie->duration;
                  movie->current = frame;
                  movie->duration += frame->duration;
                  
                  frame = NULL;
                  line = -1;
                }
            }            
        }
    }

  movie->current = NULL;
  return 0;

 blerror:
  fprintf (stderr, "Error parsing BlinkenLights movie '%s' (line %d).\n", 
           movie->filename, lc);
  if (frame)
    free (frame);
  fclose (file);
  return 1;  
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

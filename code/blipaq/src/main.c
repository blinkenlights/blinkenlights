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
#include <unistd.h>

#include <directfb.h>

#include "blipaqtypes.h"
#include "display.h"
#include "movie.h"
#include "progress.h"


int
main (int   argc,
      char *argv[])
{
  Movie      *movie;
  MovieFrame *frame;
  Display    *display;
  char       *filename = NULL;

  DisplayRotation rot = DEFAULT_ROTATION; 

  int i, timeout;
  int running = TRUE;
  int quit    = FALSE;
  int version = FALSE;
  int help    = FALSE;

  if (DirectFBInit (&argc, &argv) != DFB_OK)
    return 1;

  for (i = 1, help = FALSE, version = FALSE; 
       i < argc && !help && !version; 
       i++)
    {
      if (argv[i][0] == '-' && argv[i][1] == '-')
        {
          if (strcmp (argv[i], "--rotate") == 0)
            {
              if (++i >= argc)
                help = TRUE;
              else if (strcmp (argv[i], "left") == 0)
                rot = ROTATE_LEFT;
              else if (strcmp (argv[i], "right") == 0)
                rot = ROTATE_RIGHT;
              else if (strcmp (argv[i], "none") == 0)
                rot = ROTATE_NONE;
              else
                help = TRUE;
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
      printf ("blipaq version " VERSION "\n");
      return 0;
    }

  if (help || !filename)
    {
      printf ("blipaq version " VERSION " - ");
      printf ("A simple BlinkenLights Movie Viewer for the IPAQ\n\n");
      printf ("Usage: blipaq [options] <filename>\n\n");
      printf (" --help                       output usage information and exit\n");
      printf (" --rotate [left|right|none]   rotate screen\n");
      printf (" --version                    output version information and exit\n");
      return 0;
    }

  if (access (filename, R_OK))
    {
      fprintf (stderr, "Can't open '%s' -- aborting.\n", filename);
      return 2;
    }

  display = display_new (rot);
  if (!display)
    {
      fprintf (stderr, "Can't open display -- aborting.\n");
      return 3;
    }

  movie = movie_new (filename);

  if (!movie)
    {
      display->Release (display);
      return 4;
    }

  frame = movie->NextFrame (movie, &timeout);
  if (!frame || movie->duration < 1)
    {
      movie->Release (movie);
      display->Release (display);
      return 5;
    }    

  while (!quit)
    {
      DFBInputEvent event;

      if (!frame && running)
        {
          movie->Rewind (movie);
          frame = movie->NextFrame (movie, &timeout);
        }

      if (frame)
        {
          display->ShowFrame (display, frame);
          display->SetProgress (display, ((frame->start << PROGRESS_SHIFT) / 
                                          movie->duration));
          display->Flip (display);
        }

      if (running)
        {
          display->events->WaitForEventWithTimeout (display->events,
                                                    timeout / 1000, 
                                                    timeout % 1000);
        }
      else
        {
          display->events->WaitForEvent (display->events);
        }

      if (display->events->HasEvent (display->events) == DFB_OK)
        {
          display->events->GetEvent (display->events, DFB_EVENT (&event));

          if (event.type == DIET_KEYPRESS)
            switch (DFB_LOWER_CASE (event.key_symbol))
              {
              case DIKS_ESCAPE:
              case 'q':
              case DIKS_POWER:
              case DIKS_CALENDAR:
              case DIKS_DIRECTORY:
              case DIKS_MAIL:
              case DIKS_BACK:
                quit = TRUE;
                break;
                
              case DIKS_CURSOR_RIGHT:
              case DIKS_NEXT:
                frame = movie->NextFrame (movie, &timeout);
                break;
                
              case DIKS_CURSOR_LEFT:
              case DIKS_PREVIOUS:
                frame = movie->PreviousFrame (movie, &timeout);
                break;
                
              case DIKS_CURSOR_UP:
              case DIKS_PLAYPAUSE:
              case DIKS_SPACE:
                running = !running;
                break;
                
              case DIKS_PLAY:
                running = TRUE;
                break;
                
              case DIKS_PAUSE:
                running = FALSE;
                break;
                
              default:
                break;
              }
          
          display->events->Reset (display->events);
        }
      else /* no events pending */
        {
          if (running)
            frame = movie->NextFrame (movie, &timeout);
        }
    }

  display->Release (display);
  movie->Release (movie);

  return 0;
}

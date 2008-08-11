/* blmplay.c
 * Plays BlinkenLights movies on the text console.
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
#include <sys/time.h>

#include "blutils.h"

static void
mysleep (long pause)
{
  static struct timeval last = { 0, 0 };
  struct timeval now;
  long           expired;

  pause *= 1000;

  gettimeofday (&now, NULL);
  if (last.tv_sec != 0)
    expired = ((now.tv_sec - last.tv_sec) * 1000000 + 
               (now.tv_usec - last.tv_usec));
  else
    expired = 0;
                                                      
  if (pause - expired > 0)
    usleep (pause - expired);

  gettimeofday (&last, NULL);
}

int
main (int   argc,
      char *argv[])
{
  FILE *blm = NULL;
  char *filename = NULL;
  char  buf[4096];
  int   lc;
  int   len, i;
  int   duration;

  if (argc != 2)
    {
      fprintf (stderr, "blmplay version %s\n", VERSION);
      fprintf (stderr, "Usage: blmplay <filename>\n");
      return -1;
    }

  filename = argv[1];

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

  while (bl_fgets (buf, sizeof (buf), blm) && lc++)
    {
      len = strlen (buf);

      if (len == 0)
        continue;

      if (buf[0] == '#')
        continue;

      if (buf[0] == '@')
        {
          fflush (stdout);
          mysleep (duration);

          if (sscanf (buf+1, "%d", &duration) != 1 || duration < 0)
            duration = 0;

          printf ("\033[2J\033[H");
        }
      else
        {
          /* skip empty lines */
          for (i = 0; i < len; i++)
            if (!isspace (buf[i]))
              break;
          if (i == len)
            continue;

          /* special case last line */
          if (feof (blm))
            len++;

          for (i = 0; i < len - 1; i++)
            putchar (buf[i] == '0' ? ' ' : '#'); 

          putchar ('\n');
        }
    }
            
  fclose (blm);
  mysleep (duration);
  return 0;

 blerror:
  fprintf (stderr, "Error parsing BlinkenLights movie '%s' (line %d).\n", 
           filename, lc);
  fclose (blm);
  return -1;  
}

/* blccc - BlinkenLigths Chaos Control Center
 *
 * Copyright (c) 2001  Sven Neumann <sven@gimp.org>
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
#include <unistd.h>
#include <errno.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <gtk/gtk.h>

#include "bltypes.h"
#include "blccc.h"
#include "blisdn.h"
#include "blmovielist.h"
#include "blpong.h"
#include "bltheater.h"
#include "main-window.h"

int
main (int   argc,
      char *argv[])
{
  struct sockaddr_in  addr;
  struct hostent     *dest;
  gint               *socks;
  gint                n, i;
  BlIsdn             *isdn;
  BlPong             *pong;
  BlMovieList        *list;
  BlTheater          *theater;
  gchar              *current_dir;

  if (argc < 2)
    {
      printf ("Usage: %s hostname ...\n\n", argv[0]);
      return -1;
    }

  gtk_init (&argc, &argv);

  if (argc < 2)
    {
      printf ("Usage: %s hostname ...\n\n", argv[0]);
      return -1;
    }  

  socks = calloc (argc - 1, sizeof (int));

  for (i = 1, n = 0; i < argc; i++)
    {
      /* prepare the sockets */
      dest = gethostbyname (argv[i]);
      if (dest)
        {
          socks[n] = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
          if (socks[n] < 0)
            {
              fprintf (stderr, "Couldn't create socket for %s: %s\n", 
                       argv[i], strerror (errno));
              continue;
            }
          
          addr.sin_family = dest->h_addrtype;
          memcpy(&addr.sin_addr.s_addr, dest->h_addr_list[0], dest->h_length);
          addr.sin_port = htons (PORT);
          
          if (connect (socks[n], (struct sockaddr *) &addr, sizeof (addr)) < 0)
            {
              fprintf (stderr, "Couldn't connect socket for %s: %s\n", 
                       argv[i], strerror (errno));
              close (socks[n]);
            }
          n++;
        }
      else
        {
          fprintf (stderr, "Couldn't get name for host '%s': %s\n",
                   argv[i], hstrerror (h_errno));
        }
    }

  if (n == 0)
    return -1;

  isdn = bl_isdn_new (PONG_PORT);
  if (isdn)
    pong = bl_pong_new (isdn, WIDTH, HEIGHT);
  else
    pong = NULL;

  current_dir = g_get_current_dir ();
  list = bl_movie_list_new (current_dir);
  g_free (current_dir);

  theater = bl_theater_new (WIDTH, HEIGHT, socks, n, pong);

  main_window_create (theater, list);

  gtk_main ();

  gtk_object_sink (GTK_OBJECT (theater));
  gtk_object_sink (GTK_OBJECT (list));

  for (i = 0; i < n; i++)
    close (socks[i]);
  free (socks);

  return 0;
}

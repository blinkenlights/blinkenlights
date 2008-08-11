/* bltest - tests BlinkenLights installation
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
#include <ctype.h>
#include <unistd.h>
#include <errno.h>

#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define WIDTH  18
#define HEIGHT 8
#define PORT   2323

typedef struct _Packet Packet;
struct _Packet
{
  u_int32_t frame_magic;
  u_int32_t frame_count;
  u_int16_t frame_width;
  u_int16_t frame_height;
  u_int8_t  frame_data[HEIGHT][WIDTH];
};


static Packet packet;
static int    frame_count;


static void
test_lights (int *socks,
             int  n_socks)
{
  int  x, y, i;

  for (y = 0; y < HEIGHT; y++)
    {
      for (x = 0; x < WIDTH; x++)
	{
	  memset (packet.frame_data, '\0', sizeof (packet.frame_data));
	  packet.frame_data[y][x] = '1';
	  packet.frame_count = htonl (frame_count++);
	  for (i = 0; i < n_socks; i++)
	    send (socks[i], &packet, sizeof (Packet), 0);
	  printf ("Lampe %i.%i an... [RETURN]", 12 - y, x + 1);
	  while (getchar() != '\n');
	}
    }
}

int 
main (int   argc,
      char *argv[])
{
  struct sockaddr_in  addr;
  struct hostent     *dest;
  int                *socks;
  int                 n;
  int                 i;

  if (argc < 2)
    {
      printf ("Usage: %s hostname ...\n\n", argv[0]);
      printf ("Tests the lights on all given hosts.\n");
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

  /* prepare the packet */
  packet.frame_magic  = htonl (0xDEADBEEF);
  packet.frame_count  = htonl (0);
  packet.frame_width  = htons (WIDTH);
  packet.frame_height = htons (HEIGHT);
  frame_count = 0;

  test_lights (socks, n);

  for (i = 0; i < n; i++)
    close (socks[i]);
  free (socks);

  return 0;
}

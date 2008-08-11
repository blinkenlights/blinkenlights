/*
   windowmatrixd - receives blinkenframes from the network

   Copyright (C) 2001, 2002 Sebastian Klemke

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2, or (at your option)
   any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/


#include "common.h"


/* local variables */

static const char rcsid[] = "$Id: allon.c,v 1.4 2002/02/27 22:07:21 packet Exp $";


int
main (int argc, char ** argv)
{
  bl_frame_t            frame;
  int                   sock,
                        i;
  struct sockaddr_in    addr;

  if ((argc != 2) || (!(inet_aton (argv[1], &(addr.sin_addr))))) {
    fprintf (stderr, "You must supply the address of the windowmatrixd.\n");
    exit (23);
  }

  addr.sin_port   = htons (PORT);
  addr.sin_family = PF_INET;

  if (!(sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP))) {
    fprintf (stderr, "Failed to open socket, exiting\n");
    exit (23);
  }

  if ((setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof (i))) == -1) {
      fprintf (stderr, "Error while setting up socket.\n");
      close (sock);
      exit (23);
  }

  frame.frame_width  = htons (WIDTH);
  frame.frame_height = htons (HEIGHT);
  frame.frame_magic  = htonl (MAGIC);
  frame.frame_count  = htonl (0);
  memset (frame.frame_data, 0xFF, sizeof (frame.frame_data));

  if ((sendto (sock, (void *) &frame, sizeof (frame), 0, \
	       (struct sockaddr *) &addr, sizeof (addr))) == -1) {
    fprintf (stderr, "Error while sending packet.\n");
    close (sock);
    exit (23);
  }

  close (sock);
  exit (0);
}

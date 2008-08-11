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

static const char rcsid[] = "$Id: network.c,v 1.12 2002/02/27 22:07:21 packet Exp $";
/* static struct sockaddr_in6 addr; */


/* setup_socket
This function creates the socket we are going to use and binds it to a port. It
returns -1 on failure and the filedescriptor on success. setup_socket should be
called after setting up the output devices as the socket will be able to
receive messages right afterwards */

char
setup_socket (int *  sock, struct sockaddr_in * addr)
{
  int                i;

  if ((*sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) != -1) {
    i = 1;
    if ((setsockopt (*sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof (i))) != -1) {
      addr->sin_family = PF_INET;
      if (((bind (*sock, (struct sockaddr *) addr, sizeof (*addr))) != -1)
	  && (fcntl (*sock, F_SETFL, O_NONBLOCK) != -1)) {
	return ((char) 1);
      }
    }
    close (*sock);
  }
  return ((char) 0);
}


/* close_socket
This function closes the socket */

void
close_socket (int * sock)
{
  close (*sock);
}


/* get_frame
This function reads a frame from a given socket, which must be bound to an
address. It blocks the program until a new frame arrives, unless the socket is
nonblocking. The function returns 1 for success and saves the frame in the
given buffer. On failure, the function returns 0, the buffer might then contain
garbage */

inline char
get_frame (const int sock, struct sockaddr_in * addr, bl_frame_t * frame)
{
  char      *  buffer = NULL;
  int          size;
  socklen_t    fromlen;

  fromlen = sizeof (*addr);

  if (((size = recvfrom (sock, (void *) frame, sizeof (*frame),               \
			 0, (struct sockaddr *) addr, &fromlen)) ==           \
       sizeof (bl_frame_t))) {
    frame->frame_magic = ntohl (frame->frame_magic);
    frame->frame_width = ntohs (frame->frame_width);
    frame->frame_height = ntohs (frame->frame_height);

    if ((frame->frame_magic == MAGIC) || (frame->frame_width == WIDTH) ||     \
	(frame->frame_height == HEIGHT)) {
      frame->frame_count = ntohl (frame->frame_count);
      return ((char) 1);
    }
  }
  buffer = inet_ntoa (addr->sin_addr);
  log_printf (LOG_ERR, "Received invalid packet from %s: "                    \
	      "Magic:  0x%08X (0x%08X), "                                     \
	      "Width:  0x%04hX (0x%04hX), "                                   \
	      "Height: 0x%04hX (0x%04hX), "                                   \
	      "Size:   %i (%lu).",                                            \
	      buffer, frame->frame_magic, MAGIC, frame->frame_width,          \
	      WIDTH, frame->frame_height, HEIGHT, size,                       \
	      (unsigned long) sizeof (*frame));
  return ((char) 0);
}


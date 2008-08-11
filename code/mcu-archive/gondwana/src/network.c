/* gonwdwana - a simple bushfire simulator
 * Bushfire is a Blinkenlights Installation (TM)
 *
 * Copyright (c) 2002  Sven Neumann <sven@gimp.org>
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

#include <sys/socket.h>

#include <netinet/in.h>

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>

#include "../../mcud/mcud.h"

#include "magic-values.h"
#include "network.h"


typedef struct 
{
  mcu_frame_header_t  header;
  u_char              data[HEIGHT][WIDTH];
} Frame;

static Frame  frame;
static Frame  dummy;

int
setup_socket (struct sockaddr_in *addr)
{
  int  sock;

  sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);

  if (sock < 0)
    return -1;

  addr->sin_addr.s_addr = INADDR_ANY;
  addr->sin_port = htons (MCU_LISTENER_PORT);
  addr->sin_family = PF_INET;

  if ((bind (sock, (struct sockaddr *) addr, sizeof (*addr)) == -1)
      || (fcntl (sock, F_SETFL, O_NONBLOCK) == -1))
    {
      close (sock);
      return -1;
    }

  return sock;
}

const unsigned char *
get_frame (int                 sock, 
           struct sockaddr_in *addr)
{
  unsigned char *data = NULL;
  int            size;
  socklen_t      fromlen;

  fromlen = sizeof (*addr);

  if ((size = recvfrom (sock,
                        &frame, sizeof (Frame), 0,
                        (struct sockaddr *) addr, &fromlen)) == sizeof (Frame))
    {
      frame.header.magic  = ntohl (frame.header.magic);

      if (frame.header.magic == MAGIC_MCU_FRAME)
        {
          frame.header.width  = ntohs (frame.header.width);
          frame.header.height = ntohs (frame.header.height);

          if (frame.header.width == WIDTH && frame.header.height == HEIGHT)
            {
              data = (unsigned char *) &frame.data;
            }
#ifdef DEBUG
          else
            {
              fprintf (stderr, "Incorrect frame-header:\n"
                       "  Magic:  0x%08X, expected 0x%08X\n"
                       "  Width:  0x%04hX, expected 0x%04hX\n"
                       "  Height: 0x%04hX, expected 0x%04hX\n",
                       frame.header.magic, MAGIC_MCU_FRAME,
                       frame.header.width, WIDTH, frame.header.height, HEIGHT);
            }
#endif /* DEBUG */
        }
      else if (frame.header.magic == MAGIC_HDL_FRAME ||
               frame.header.magic == MAGIC_HDL2_FRAME)
        {
          /*  backward compatibility mode for HDL 1 & 2 frames.
           *  coincidentially, the headers have the same length
           */
          bl_frame_header_t *hdl_header;

          hdl_header = (bl_frame_header_t *) &frame.header;

          hdl_header->frame_width  = ntohs (hdl_header->frame_width);
          hdl_header->frame_height = ntohs (hdl_header->frame_height);

          if (hdl_header->frame_width == WIDTH &&
              hdl_header->frame_height == HEIGHT)
            {
              data = (unsigned char *) &frame.data;

              if (frame.header.magic == MAGIC_HDL_FRAME)
                {
                  int i;

                  for (i = 0; i < WIDTH * HEIGHT; i++)
                    data[i] *= 255;
                }
            }
#ifdef DEBUG
          else
            {
              fprintf (stderr, "Incorrect HDL frame-header:\n"
                       "  Magic:  0x%08X, expected 0x%08X or 0x%08X\n"
                       "  Width:  0x%04hX, expected 0x%04hX\n"
                       "  Height: 0x%04hX, expected 0x%04hX\n",
                       frame.header.magic, MAGIC_HDL_FRAME, MAGIC_HDL2_FRAME,
                       hdl_header->frame_width, WIDTH,
                       hdl_header->frame_height, HEIGHT);
            }
#endif /* DEBUG */
        }
    }
#ifdef DEBUG
  else
    {
      fprintf (stderr, "Incorrect frame length: %d, expected %d",
               size, sizeof (Frame));
    }
#endif /* DEBUG */

  while (read (sock, &dummy, sizeof (Frame)) >= 0)
    ;

  return data;
}

const unsigned char *
get_test_frame (void)
{
  unsigned char *data;
  int            i;

  data = (unsigned char *) &frame.data;

  for (i = 0; i < WIDTH * HEIGHT; i++)
    *data++ = ((i + 1) << 4) - 1;

  return (unsigned char *) &frame.data;
}


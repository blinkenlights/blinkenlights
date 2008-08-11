/* blplay - play BlinkenLights movie files
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


static inline char *
fget_line (char *s, int size, FILE *stream)
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

static void
play_frames (int *socks,
             int  n_socks)
{
  char buf[1024];
  int  i;
  int  len;
  int  line;
  int  duration;

  line = -1;

  while (fget_line (buf, sizeof (buf), stdin))
    {
      len = strlen (buf);
      
      /* output comments */
      if (buf[0] == '#')
        {
          if (len > 0)
            printf ("   %s", buf + 1);
          continue;
        }

      /* skip empty lines */
      for (i = 0; i < len; i++)
        if (!isspace (buf[i]))
          break;
      if (i == len)
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
          if (buf[0] == '@' || len - 1 < WIDTH) /* one for linefeed */
            {
              fprintf (stderr, "Invalid frame, skipping.\n");
              line = -1;
            }
          else
            {
              for (i = 0; i < WIDTH; i++)
                packet.frame_data[line][i] = (buf[i] == '1' ? 1 : 0);

              if (++line == HEIGHT)
                {
                  packet.frame_count = htonl (frame_count++);
                  for (i = 0; i < n_socks; i++)
                    send (socks[i], &packet, sizeof (Packet), 0);
                  usleep (duration * 1000);
                  line = -1;
                }
            }
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
      printf ("Reads from stdin and sends to all given hosts.\n");
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

  play_frames (socks, n);

  for (i = 0; i < n; i++)
    close (socks[i]);
  free (socks);

  return 0;
}

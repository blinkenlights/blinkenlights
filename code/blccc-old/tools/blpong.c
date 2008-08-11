/* blpong - play Pong on BlinkenLights
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

#define WIDTH   18
#define HEIGHT  8
#define PORT    2323
#define SPEED  (300 * 1000)

#define CLAMP(x,l,u) ((x) < (l) ? (l) : ((x) > (u) ? (u) : (x))) 

typedef enum
{
  RIGHT,
  LEFT
} XDirection;

typedef enum
{
  UP,
  DOWN
} YDirection;

typedef enum
{
  HUMAN,
  COMPUTER
} PlayerType;

static int            lpaddle;
static int            rpaddle;
static int            ball_x;
static int            ball_y;
static XDirection     ball_xdir;
static YDirection     ball_ydir;


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
draw_screen (int *socks,
             int  n_socks,
             int  ball_x,
             int  ball_y,
             int  lpaddle,
             int  rpaddle)
{
  int i;

  memset (&packet.frame_data, 0, WIDTH * HEIGHT);

  if (lpaddle >= 0 && lpaddle < HEIGHT - 2)
    {
      packet.frame_data[lpaddle    ][0] = 1;
      packet.frame_data[lpaddle + 1][0] = 1;
      packet.frame_data[lpaddle + 2][0] = 1;
    }
  
  if (rpaddle >= 0 && rpaddle < HEIGHT - 2)
    {
      packet.frame_data[rpaddle    ][WIDTH - 1] = 1;
      packet.frame_data[rpaddle + 1][WIDTH - 1] = 1;
      packet.frame_data[rpaddle + 2][WIDTH - 1] = 1;
    }

  if (ball_x >= 0 && ball_x < WIDTH &&
      ball_y >= 0 && ball_y < HEIGHT)
    {
      packet.frame_data[ball_y][ball_x] = 1;
    }

  packet.frame_count = htonl (frame_count++);
  for (i = 0; i < n_socks; i++)
    send (socks[i], &packet, sizeof (Packet), 0);
}

static void
init_game (void)
{
  int foo;

  lpaddle = HEIGHT / 2 - 1;
  rpaddle = HEIGHT / 2 - 1;

  foo = rand();

  if (foo & 0x1)
    {
      ball_x = 0;
      ball_xdir = RIGHT;
    }
  else
    {
      ball_x = WIDTH - 1;
      ball_xdir = LEFT;
    }

  if (foo & 0x2)
    {
      ball_y = 0;
      ball_ydir = DOWN;
    }
  else
    {
      ball_y = HEIGHT - 1;
      ball_ydir = UP;
    }
}

static int
reflect (int paddle_y)
{
  switch (ball_ydir)
    {
    case DOWN: /* we hit the paddle coming from the top */
      switch (ball_y - paddle_y)
        {
        case 0:
          ball_ydir = UP;
          ball_y -= 2;
          break;
        case 1:
        case 2:
        case 3:
          break;
        default:
          return 0;
        }
      break;

    case UP: /* we hit the paddle coming from the bottom */
      switch (ball_y - paddle_y)
        {
        case -1:
        case 0:
        case 1:
          break;
        case 2:
          ball_ydir = DOWN;
          ball_y += 2;
          break;
        default:
          return 0;
        }
      break;          
    }

  return 1;
}

static int
move_ball (void)
{
  switch (ball_xdir)
    {
    case RIGHT:
      ball_x++;
      break;
    case LEFT:
      ball_x--;
      break;
    }
  switch (ball_ydir)
    {
    case UP:
      ball_y--;
      break;
    case DOWN:
      ball_y++;
      break;
    }

  /* collision with walls ? */
  if (ball_y < 0)
    {
      ball_y = 1;
      ball_ydir = DOWN;
    }
  else if (ball_y >= HEIGHT)
    {
      ball_y = HEIGHT - 2;
      ball_ydir = UP;
    }
  
  /* collision with left paddle or out ? */
  if (ball_x == 0)
    {
      if (!reflect (lpaddle))
        return 1;  /* right wins */

      ball_x = 2;
      ball_xdir = RIGHT;
    }
  /* collision with right paddle or out ? */
  else if (ball_x == WIDTH - 1)
    {
      if (!reflect (rpaddle))
        return -1;  /* left wins */
      ball_x = WIDTH - 3;
      ball_xdir = LEFT;
    }

  return 0;
}

static void
computer_move (int *paddle)
{
  if (rand() & 1)
    return;

  if (*paddle - ball_y > -1)
    (*paddle)--;
  else if (*paddle - ball_y < 1)
    (*paddle)++;
  
  *paddle = CLAMP (*paddle, 0, HEIGHT - 3);
}

static void
play_game (int *socks,
           int  n_socks)
{
  draw_screen (socks, n_socks, -1, -1, lpaddle, rpaddle);
  usleep (2 * SPEED);

  while (1)
    {
      draw_screen (socks, n_socks, ball_x, ball_y, lpaddle, rpaddle);

      usleep (SPEED);

      computer_move (&lpaddle);
      computer_move (&rpaddle);

      if (move_ball ())
        {
          int i;

          for (i = 0; i < 4; i++)
            {
              draw_screen (socks, n_socks, ball_x, ball_y, lpaddle, rpaddle);
              usleep (SPEED);
              draw_screen (socks, n_socks, -1, -1, lpaddle, rpaddle);
              usleep (SPEED);
            }
          
          draw_screen (socks, n_socks, -1, -1, -1, -1);
          usleep (3 * SPEED);
          break;
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
  packet.frame_magic  = htonl (0xDEADBEFF);
  packet.frame_count  = htonl (0);
  packet.frame_width  = htons (WIDTH);
  packet.frame_height = htons (HEIGHT);
  frame_count = 0;

  while (1)
    {
      init_game ();
      play_game (socks, n);
    }

  for (i = 0; i < n; i++)
    close (socks[i]);
  free (socks);

  return 0;
}

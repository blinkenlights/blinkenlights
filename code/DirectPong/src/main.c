/* DirectPong
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
#include <unistd.h>

#include "gfx.h"
#include "utils.h"


#define X_RES        18
#define Y_RES         8
#define DEFAULT_FPS   3

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

static PlayerType     lplayer = COMPUTER;
static PlayerType     rplayer = COMPUTER;

static int            lscore;
static int            rscore;

static unsigned long  speed   = (1000000 / DEFAULT_FPS);


static void
init_game (void)
{
  int foo;

  lpaddle = Y_RES / 2 - 1;
  rpaddle = Y_RES / 2 - 1;

  foo = rand();

  if (foo & 0x1)
    {
      ball_x = 0;
      ball_xdir = RIGHT;
    }
  else
    {
      ball_x = X_RES - 1;
      ball_xdir = LEFT;
    }

  if (foo & 0x2)
    {
      ball_y = 0;
      ball_ydir = DOWN;
    }
  else
    {
      ball_y = Y_RES - 1;
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
  else if (ball_y >= Y_RES)
    {
      ball_y = Y_RES - 2;
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
  else if (ball_x == X_RES - 1)
    {
      if (!reflect (rpaddle))
        return -1;  /* left wins */
      ball_x = X_RES - 3;
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
  
  *paddle = CLAMP (*paddle, 0, Y_RES - 3);
}

static int
play_game (void)
{
  char buf[8];
  int  winner;

  draw_game_screen (-1, -1, lpaddle, rpaddle);
  usleep (2 * speed);

  while (1)
    {
      draw_game_screen (ball_x, ball_y, lpaddle, rpaddle);

      usleep (speed);

      if (check_events (lplayer == HUMAN ? &lpaddle : NULL,
                        rplayer == HUMAN ? &rpaddle : NULL))
        break;

      if (lplayer == COMPUTER)
        computer_move (&lpaddle);
      if (rplayer == COMPUTER)
        computer_move (&rpaddle);

      winner = move_ball ();
      if (winner)
        {
          int i;

          if (winner > 0)
            rscore++;
          else
            lscore++;

          for (i = 0; i < 4; i++)
            {
              draw_game_screen (ball_x, ball_y, lpaddle, rpaddle);
              usleep (speed);
              draw_game_screen (-1, -1, lpaddle, rpaddle);
              usleep (speed);
            }
          
          /* our font is to large for scores > 9 */
          if (lscore > 9 || rscore > 9)
            lscore = rscore = 0;

          snprintf (buf, sizeof (buf), "%d:%d", lscore, rscore); 
          if (draw_text_screen (buf))
            sleep (1);

          draw_empty_screen ();
          usleep (10 * speed);

          return 1;
        }
    }

  return 0;
}  

int
main (int   argc,
      char *argv[])
{
  const char *gfx_engine = "house";
  int i;

  for (i = 1; i < argc; i++)
    {
      if (strcmp (argv[i], "-0") == 0)
        {
          lplayer = COMPUTER;
          rplayer = COMPUTER;
        }
      if (strcmp (argv[i], "-1") == 0)
        {
          lplayer = COMPUTER;
          rplayer = HUMAN;
        }
      else if (strcmp (argv[i], "-2") == 0)
        {
          lplayer = HUMAN;
          rplayer = HUMAN;
        }
      else if (strcmp (argv[i], "--house") == 0 || 
               strcmp (argv[i], "-h") == 0)
        {
          gfx_engine = "house";
        }
      else if (strcmp (argv[i], "--simple") == 0 || 
               strcmp (argv[i], "-s") == 0)
        {
          gfx_engine = "simple";
        }
      else if (strcmp (argv[i], "--fps") == 0 || 
               strcmp (argv[i], "-f") == 0)
        {
          int fps;

          if (i + 1 < argc && sscanf (argv[i+1], "%d", &fps) == 1)
            {
              i++;
              if (fps > 0)
                speed = 1000000 / fps;
            }
        }
      else if (strcmp (argv[i], "--help") == 0 ||
               strcmp (argv[i], "-?") == 0)
        {
          printf ("\nDirectPong version %s\n\n", VERSION);
          printf ("Usage: %s [options]\n\n", argv[0]);
          printf ("Options: \n"
                  "  -0             no human players (default)\n"
                  "  -1             one human player\n"
                  "  -2             two human players\n"
                  "  -h, --house    draw game on house (default)\n"
                  "  -s, --simple   draw simple console gfx\n"
                  "  -f, --fps <n>  number of frames per second (default %d)\n"
                  "\n", DEFAULT_FPS);
          exit (-1);
        }
    }

  if (! init_gfx (&argc, &argv, Y_RES, X_RES, gfx_engine))
    exit (-1);

  lscore = rscore = 0;
  do 
    {
      init_game ();
    }
  while (play_game ());

  release_gfx ();

  return 0;
}

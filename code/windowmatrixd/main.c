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

static const char    rcsid[]  = "$Id: main.c,v 1.20 2002/02/27 22:07:21 packet Exp $";
static       char  * progname = NULL;


/* global functions */

int main (int, char **);


/* local functions */

static void version (void);
static void usage (const int);


/* main
does all there is to it */

int
main (int argc, char ** argv)
{
  bl_frame_t             frame;
  int                    sock,
                         result,
                         c;
  device_t            *  devices;
  unsigned int           current = 0,
                         expected;
  struct sockaddr_in     addr;
  fd_set                 set;
  struct timeval         tv;
  char                *  buffer;
  unsigned short         interactivity = 0;
  const struct option    long_options[] = {
    {"help",        no_argument,       NULL, 'h'},
    {"version",     no_argument,       NULL, 'V'},
    {"interface",   required_argument, NULL, 'i'},
    {"debug",       no_argument,       NULL, 'D'},
    {NULL,          0,                 NULL, 0}
  };

  addr.sin_addr.s_addr = INADDR_ANY;

  if (!(progname = strdup (basename (argv[0])))) {
    log_printf (LOG_ERR, "Could not copy program name, exiting.");
    exit (EXIT_FAILURE);
  }

  while ((c = getopt_long (argc, argv, "hVi:D", long_options, NULL)) != -1) {
    switch (c) {
    case 'V':
      version ();
      break;
    case 'h':
      usage (EXIT_SUCCESS);
      break;
    case 'D':
      interactivity = 1;
      break;
    case 'i':
      if (!(inet_aton (optarg, &(addr.sin_addr)))) {
	fprintf (stderr, "Bad Argument %s, expecting IPv4 Address.\n\n",      \
		 optarg);
	usage (EXIT_FAILURE);
      }
    }
  }

  if (!interactivity) {
    if ((daemon (0, 0))) {
      /*  FIXME: This error-message might in some circumstances not be
	  shown. */
      log_printf (LOG_ERR, "Couldn't detach process from parent, exiting.");
      exit (EXIT_FAILURE);
    }
  }

  if (!(setup_log (interactivity, progname))) {
    log_printf (LOG_ERR, "Unable to open log, exiting.");
    exit (EXIT_FAILURE);
  }

  log_printf (LOG_INFO, "windowmatrixd version %s started, using port %hi, "  \
	      "height %hi, width %hi, magic 0x%08X", VERSION, PORT, HEIGHT,   \
	      WIDTH, MAGIC);

  if (!(devices = setup_devices ())) {
    log_printf (LOG_ERR, "Failed to open devices, exiting.");
    exit (EXIT_FAILURE);
  }

  addr.sin_port = htons (PORT);

  if (setup_socket (&sock, &addr)) {
    FD_ZERO (&set);
    FD_SET (sock, &set);
    tv.tv_sec  = BLANK_TIMEOUT;
    tv.tv_usec = 0;
          
    while ((result = select (sock+1, &set, NULL, NULL, &tv)) >= 0 ||
	   errno == EINTR) {
      FD_ZERO(&set);
      FD_SET(sock,&set);
      tv.tv_sec  = BLANK_TIMEOUT;
      tv.tv_usec = 0;
              
      if (result < 0)  /* select was interupted */
	continue;
              
      if (result == 0) { /* select timed out */
	frame.frame_width  = WIDTH;
	frame.frame_height = HEIGHT;
	memset (&frame.frame_data, 0, WIDTH * HEIGHT);
	output_frame (&frame, devices);
	continue;
      }
      
      if (get_frame (sock, &addr, &frame)) {
	expected = (current == 0xFFFFFFFFL) ? 0 : current + 1;
	current = frame.frame_count;

	if (current != expected) {
	  buffer = inet_ntoa (addr.sin_addr);
	  log_printf (LOG_WARNING, "Received unexpected packet from "         \
		      "%s, Expected-Count: %u, Real-Count: %u.",              \
		      buffer, expected, current);
	}

	output_frame (&frame, devices);
      }
    }
    close_socket (&sock);
  }
  log_printf (LOG_ERR, "Failed to open socket, exiting.");

  close_devices (devices);
  devices = NULL;

  close_log();

  free (progname);

  exit (EXIT_FAILURE);
}


/* usage
displays usage information and exit */

static void
usage (const int status)
{
  fprintf (status ? stderr : stdout, "Usage: %s [OPTION]...\n"                \
	  "Receive blinkenframes and output via pio-cards.\n\n"               \
	  "  -h, --help                 display this help and exit\n"         \
	  "  -V, --version              output version information and exit\n"\
	  "  -i ADDR, --interface=ADDR  bind to interface with address ADDR\n"\
	  "  -D, --debug                run in foreground\n\n"                \
	  "Report bugs to <packet@berlin.ccc.de>.\n", progname);
  exit (status);
}


/* version
displays version information and exits */

static void
version (void)
{
  printf ("windowmatrixd %s\n"                                                \
	  "Copyright (C) 2002 Sebastian Klemke\n"                             \
	  "This is free software; see the source for copying conditions.  "   \
	  "There is NO\nwarranty; not even for MERCHANTABILITY or FITNESS "   \
	  "FOR A PARTICULAR PURPOSE.\n", VERSION);
  exit (EXIT_SUCCESS);
}

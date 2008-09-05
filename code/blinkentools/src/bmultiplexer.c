/* bmultiplex - receive udp packets and forward them to multiple hosts
 *
 * Copyright (C) 2002  The Blinkenlights Crew
 *                     Daniel Mack <daniel@yoobay.net>
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

#include <stdlib.h>
#include <string.h>

#include "blib/blib.h"

#define HASH 0

static gint width    = -1;
static gint height   = -1;
static gint channels = -1;
static gint maxval   = -1;


static gboolean
callback (BReceiver *rec,
          BPacket   *packet,
          gpointer   callback_data)
{
  BSender *sender = B_SENDER (callback_data);

  if (packet->header.mcu_frame_h.width    != width    ||
      packet->header.mcu_frame_h.height   != height   ||
      packet->header.mcu_frame_h.channels != channels ||
      packet->header.mcu_frame_h.maxval   != maxval)
    {
      width    = packet->header.mcu_frame_h.width;
      height   = packet->header.mcu_frame_h.height;
      channels = packet->header.mcu_frame_h.channels;
      maxval   = packet->header.mcu_frame_h.maxval;

      b_sender_configure (sender, width, height, channels, maxval);
    }

#ifdef HASH
  g_print ("#");
#endif

  b_sender_send_frame (sender, packet->data);

  return TRUE;
}


int
main (int   argc,
      char *argv[])
{
  BSender   *sender;
  BReceiver *receiver;
  GError    *error = NULL;
  gint       i, port, listen_port;
  gchar     *colon;
  GMainLoop *main_loop;

  if (argc < 3)
    {
      g_printerr ("bmultiplexer (%s version %s)\n\n", PACKAGE, VERSION);
      g_printerr ("Usage: %s <port> <hostname[:port]>+\n", argv[0]);
      return EXIT_FAILURE;
    }

  b_init ();

  sender = b_sender_new ();
  g_assert (sender);

  receiver = b_receiver_new (callback, sender);

  if (!b_parse_int (argv[1], &listen_port))
    {
      g_printerr ("Invalid listen port: '%s'\n", argv[1]);
      return EXIT_FAILURE;
    }

  if (!b_receiver_listen (receiver, listen_port))
    {
      g_printerr ("Unable to make receiver listen on port %d!\n", listen_port);
      return EXIT_FAILURE;
    }

  for (i = 2; i < argc; i++)
    {
      port = MCU_LISTENER_PORT;

      if ((colon = strchr (argv[i], ':')) != NULL)
	{
	  *colon = '\0';
	  colon++;
	  b_parse_int (colon, &port);
	}
      if (!b_sender_add_recipient (sender, -1, argv[i], port, &error))
        {
          g_printerr (error->message);
          return EXIT_FAILURE;
        }
    }

  g_print ("Waiting for udp packets arriving on port %d ...\n", listen_port);

  main_loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (main_loop);

  g_object_unref (sender);

  return EXIT_SUCCESS;
}

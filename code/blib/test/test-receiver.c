/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (C) 2002  The Blinkenlights Crew
 *                     Sven Neumann <sven@gimp.org>
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

#include <stdlib.h>

#include "blib/blib.h"


static gboolean
callback (BReceiver *rec,
          BPacket   *packet,
          gpointer   callback_data)
{
  gint w, h;

  g_print ("\033[2J\033[H");
  g_print ("width %02d, height %02d, channels %02d, maxval %03d\n",
           packet->header.mcu_frame_h.width,
           packet->header.mcu_frame_h.height,
           packet->header.mcu_frame_h.channels,
           packet->header.mcu_frame_h.maxval);
  
  for (h = 0; h < packet->header.mcu_frame_h.height; h++)
    {
      for (w = 0; w < packet->header.mcu_frame_h.width; w++)
        g_print ("%02x ", packet->data[h * packet->header.mcu_frame_h.width + w]);
      g_print ("\n");
    }
  
  return TRUE; /* returning FALSE would stop reception of packets */
}

int
main (int   argc,
      char *argv[])
{
  BReceiver *receiver;
  GMainLoop *loop;
	
  b_init ();

  g_print ("creating receiver\n");
  receiver = b_receiver_new (callback, NULL);
  g_assert (receiver);

  g_print ("making receiver listen\n");
  b_receiver_listen (receiver, 2323);
  
  g_print ("waiting for packets on port 2323, press ^C to stop\n");
  loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (loop);
  g_main_loop_unref (loop);
  loop = NULL;
  
  g_object_unref (receiver);

  return EXIT_SUCCESS;
}

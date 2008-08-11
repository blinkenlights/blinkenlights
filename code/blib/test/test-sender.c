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

int
main (int   argc,
      char *argv[])
{
  BSender *sender;
  guchar  *data;
  GError  *error = NULL;
  gint     i;

  if (argc < 2)
    {
      g_printerr ("Usage: %s <hostname>+\n", argv[0]);
      return EXIT_FAILURE;
    }

  b_init ();

  sender = b_sender_new ();
  g_assert (sender);

  for (i = 1; i < argc; i++)
    {
      if (!b_sender_add_recipient (sender, -1,
                                   argv[i], MCU_LISTENER_PORT, &error))
        {
          g_printerr (error->message);
          return EXIT_FAILURE;
        }
    }

  b_sender_configure (sender, 26, 20, 1, 255);

  data = g_new (guchar, 20 * 26);

  for (i = 0; i < 5; i++)
    b_sender_send_frame (sender, data);

  g_free (data);
  g_object_unref (sender);

  return EXIT_SUCCESS;
}

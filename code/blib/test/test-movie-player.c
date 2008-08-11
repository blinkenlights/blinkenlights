/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (C) 2002  The Blinkenlights Crew
 *                     Sven Neumann <sven@gimp.org>
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
paint (BModule  *module,
       guchar   *buffer,
       gpointer  data)
{
  b_sender_send_frame (B_SENDER (data), buffer);

  return TRUE;
}

int
main (int   argc,
      char *argv[])
{
  GMainLoop *loop;
  BModule   *module;
  BSender   *sender;
  BMovie    *movie;
  gint       i;
  GError    *error = NULL;

  if (argc < 3)
    {
      g_print ("Usage: %s <movie> <host>+\n", argv[0]);
      return EXIT_FAILURE;
    }

  b_init ();

  movie = b_movie_new_from_file (argv[1], TRUE, &error);
  if (!movie)
    {
      g_print ("Error opening '%s': %s\n", argv[1], error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }

  g_print ("Creating BSender, ");
  sender = b_sender_new ();

  b_sender_configure (sender,
                      movie->width, movie->height, movie->channels, 255);

  for (i = 2; i < argc; i++)
    b_sender_add_recipient (sender, -1, argv[i], MCU_LISTENER_PORT, NULL);

  g_print (" added %d recipients.\n", i - 2);

  g_print ("Creating a BMoviePlayer for '%s' ... ",
           b_object_get_name (B_OBJECT (movie)));

  module = b_module_new (B_TYPE_MOVIE_PLAYER,
                         movie->width, movie->height, NULL,
                         paint, sender,
                         &error);

  g_object_unref (movie);

  if (!module)
    {
      g_print ("failed: %s\n", error->message);

      g_object_unref (sender);
      return EXIT_FAILURE;
    }
  g_print ("OK.\n");

  g_print ("Preparing the BMoviePlayer module ... ");

  g_object_set (G_OBJECT (module), "movie", argv[1], NULL);

  if (!b_module_prepare (module, &error))
    {
      g_print ("failed: %s\n", error->message);

      g_object_unref (module);
      g_object_unref (sender);

      return EXIT_FAILURE;
    }
  g_print ("OK.\n");

  loop = g_main_loop_new (NULL, FALSE);

  g_signal_connect_swapped (G_OBJECT (module), "stop",
                            G_CALLBACK (g_main_loop_quit), loop);

  g_print ("Start playing ...\n");

  b_module_start (module);

  g_main_loop_run (loop);
  g_main_loop_unref (loop);

  g_print ("Movie finished, quitting.\n");

  g_object_unref (module);
  g_object_unref (sender);

  return EXIT_SUCCESS;
}

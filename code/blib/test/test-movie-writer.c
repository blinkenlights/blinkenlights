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
#include <string.h>

#include "blib/blib.h"


int
main (int   argc,
      char *argv[])
{
  BMovie *movie;
  GType   type  = G_TYPE_NONE;
  GError *error = NULL;
  
  if (argc < 3)
    {
      g_printerr ("Usage: %s <format> <filename>\n", argv[0]);
      return EXIT_FAILURE;
    }

  b_init ();

  if (g_strcasecmp (argv[1], "blm") == 0)
    type = B_TYPE_MOVIE_BLM;
  else if (g_strcasecmp (argv[1], "bml") == 0)
    type = B_TYPE_MOVIE_BML;
  else if (g_strcasecmp (argv[1], "gif") == 0)
    type = B_TYPE_MOVIE_GIF;
  else
    g_printerr ("Unknown output format, "
                "must be \"blm\", \"bml\" or \"gif\".\n");

  if (!g_type_is_a (type, B_TYPE_MOVIE))
    return EXIT_FAILURE;

  if (strcmp (argv[2], "-"))
    movie = b_movie_new_from_file (argv[2], FALSE, &error);
  else
    movie = b_movie_new_from_fd (0, &error);

  if (!movie)
    {
      g_return_val_if_fail (error != NULL, EXIT_FAILURE);

      g_printerr ("Error opening '%s': %s\n", argv[2], error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }

  if (!b_movie_save_as (movie, type, stdout, &error))
    {
      g_return_val_if_fail (error != NULL, EXIT_FAILURE);

      g_printerr ("Error writing '%s': %s\n",
                  b_object_get_name (B_OBJECT (movie)), error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }

  g_printerr ("successfully wrote '%s' (%dx%d) as %s\n",
              movie->title ? movie->title : b_object_get_name (B_OBJECT (movie)),
              movie->width, movie->height, g_type_name (type));

  g_object_unref (movie);

  return EXIT_SUCCESS;
}

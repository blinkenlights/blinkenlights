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


int
main (int   argc,
      char *argv[])
{
  BTheme *theme;
  GError *error = NULL;

  b_init ();

  if (argc < 2)
    {
      g_printerr ("Usage: %s <filename>\n", argv[0]);
      return EXIT_FAILURE;
    }

  theme = b_theme_new_from_file (argv[1], TRUE, &error);

  if (!theme)
    {
      g_return_val_if_fail (error != NULL, EXIT_FAILURE);

      g_printerr ("Error opening '%s': %s\n", argv[1], error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }

  if (!b_theme_load (theme, &error))
    {
      g_return_val_if_fail (error != NULL, EXIT_FAILURE);

      g_printerr ("Error parsing '%s': %s\n",
                  b_object_get_name (B_OBJECT (theme)), error->message);
      g_error_free (error);
      return EXIT_FAILURE;
    }

  g_print ("successfully loaded '%s' (%dx%d)\n",
           b_object_get_name (B_OBJECT (theme)), theme->width, theme->height);

  g_object_unref (theme);

  return EXIT_SUCCESS;
}

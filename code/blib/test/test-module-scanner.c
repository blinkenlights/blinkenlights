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


static void
print_info (GType  type)
{
  GParamSpec **props;
  GTypeClass  *class;
  guint        n_props;

  class = g_type_class_ref (type);

  props = g_object_class_list_properties (G_OBJECT_CLASS (class), &n_props);

  g_print ("BModule subtype: %s\n\n", g_type_name (type));

  if (! props)
    {
      g_print (" %s has no properties.\n", g_type_name (type));
    }
  else
    {
      gint i;

      g_print ("  %s has %d properties:\n", g_type_name (type), n_props);

      for (i = 0; i < n_props; i++)
        {
          GParamSpec *spec = props[i];

          g_print ("   name:   %s\n", g_param_spec_get_name (spec));
          g_print ("   blurb:  %s\n", g_param_spec_get_blurb (spec));
          g_print ("   type:   %s\n",
                   g_type_name (G_PARAM_SPEC_VALUE_TYPE (spec)));
          g_print ("\n");
        }
    }

  g_free (props);

  g_type_class_unref (class);
}

int
main (int   argc,
      char *argv[])
{
  volatile GType  movie_player_type;
  const gchar    *module_dir = "../modules";
  gint            i;

  if (argc > 1)
    module_dir = argv[1];

  b_init ();

  /* load the builtin movie player */
  movie_player_type = B_TYPE_MOVIE_PLAYER;
  g_print ("Registered the builtin BMoviePlayer module.\n");

  g_print ("Scanning %s for loadable modules ... ", module_dir);
  i = b_module_infos_scan_dir (argv[1]);
  g_print ("registered %d modules\n", i);

  if (i > 0)
    {
      GType *child_types;
      guint  n_child_types;

      child_types = g_type_children (B_TYPE_MODULE, &n_child_types);

      for (i = 0; i < n_child_types; i++)
        print_info (child_types[i]);

      g_free (child_types);
    }

  return EXIT_SUCCESS;
}

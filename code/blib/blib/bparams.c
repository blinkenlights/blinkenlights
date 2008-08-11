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

#include "config.h"

#include <glib-object.h>

#include "btypes.h"
#include "bparams.h"

static void  b_param_filename_class_init (GParamSpecClass *class);

GType
b_param_filename_get_type (void)
{
  static GType spec_type = 0;

  if (!spec_type)
    {
      static const GTypeInfo type_info = 
      {
        sizeof (GParamSpecClass),
        NULL, NULL, 
        (GClassInitFunc) b_param_filename_class_init, 
        NULL, NULL,
        sizeof (GParamSpecString),
        0, NULL, NULL
      };

      spec_type = g_type_register_static (G_TYPE_PARAM_STRING,
                                          "BParamFilename", &type_info, 0);
    }
  
  return spec_type;
}

static void
b_param_filename_class_init (GParamSpecClass *class)
{
  class->value_type = B_TYPE_FILENAME;
}

/**
 * b_param_spec_filename:
 * @name: the property name
 * @nick: an optional short version of the name
 * @blurb: an optional description
 * @default_value: the default value (may be %NULL)
 * @flags: the #GParamFlags for this param_spec
 * 
 * Creates a new #GParamSpec for a #B_TYPE_FILENAME property.
 * 
 * Return value: the newly allocate #GParamSpec.
 **/
GParamSpec *
b_param_spec_filename (const gchar *name,
                       const gchar *nick,
                       const gchar *blurb,
                       gchar       *default_value,
                       GParamFlags  flags)
{
  GParamSpecString *pspec;

  pspec = g_param_spec_internal (B_TYPE_PARAM_FILENAME,
                                 name, nick, blurb, flags);

  pspec->default_value = default_value;
  
  return G_PARAM_SPEC (pspec);
}

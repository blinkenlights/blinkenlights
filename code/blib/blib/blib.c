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

#include <stdlib.h>
#include <time.h>

#include <glib-object.h>

#include "btypes.h"
#include "butils.h"

static void  b_register_transform_funcs (void);

static gboolean initialized = FALSE;


/**
 * b_init:
 * 
 * This function initializes the BLib library. It calls g_type_init()
 * for you, initializes the random number generator and registers some
 * transform functions needed for the deserialization routines.
 *
 * It is safe to call this function multiple times but you should at
 * least call it once before you use any other BLib functions.
 **/
void
b_init (void)
{
  if (initialized)
    return;

  srand (time (NULL));
  
  g_type_init ();

  b_register_transform_funcs ();
  
  initialized = TRUE;
}

static void
b_value_transform_string_int (const GValue *src_value,
                              GValue       *dest_value)
{
  const gchar *str = g_value_get_string (src_value);
  gint         i;

  if (b_parse_int (str, &i))
    g_value_set_int (dest_value, i);
  else
    g_warning ("can not convert '%s' to an integer value", str);
}

static void
b_value_transform_string_double (const GValue *src_value,
                                 GValue       *dest_value)
{
  const gchar *str = g_value_get_string (src_value);
  gdouble      d;

  if (b_parse_double (str, &d))
    g_value_set_double (dest_value, d);
  else
    g_warning ("can not convert '%s' to a double value", str);
}

static void
b_value_transform_string_boolean (const GValue *src_value,
                                  GValue       *dest_value)
{
  const gchar *str = g_value_get_string (src_value);
  gboolean     b;

  if (b_parse_boolean (str, &b))
    g_value_set_boolean (dest_value, b);
  else
    g_warning ("can not convert '%s' to a boolean value", str);
}

static void
b_value_transform_string_enum (const GValue *src_value,
                               GValue       *dest_value)
{
  GEnumClass *enum_class;
  GEnumValue *enum_value;

  g_return_if_fail (G_VALUE_HOLDS_ENUM (dest_value));

  enum_class = g_type_class_peek (G_VALUE_TYPE (dest_value));
  enum_value = g_enum_get_value_by_name (G_ENUM_CLASS (enum_class), 
                                         g_value_get_string (src_value));
  if (!enum_value)
    enum_value = g_enum_get_value_by_nick (G_ENUM_CLASS (enum_class), 
                                           g_value_get_string (src_value));

  if (enum_value)
    g_value_set_enum (dest_value, enum_value->value);
  else
    g_warning ("can not convert '%s' to an enum value",
               g_value_get_string (src_value));
}

static void
b_register_transform_funcs (void)
{
  g_value_register_transform_func (G_TYPE_STRING, G_TYPE_INT,
                                   b_value_transform_string_int); 
  g_value_register_transform_func (G_TYPE_STRING, G_TYPE_DOUBLE,
                                   b_value_transform_string_double); 
  g_value_register_transform_func (G_TYPE_STRING, G_TYPE_BOOLEAN,
                                   b_value_transform_string_boolean); 
  g_value_register_transform_func (G_TYPE_STRING, G_TYPE_ENUM,
                                   b_value_transform_string_enum); 
}

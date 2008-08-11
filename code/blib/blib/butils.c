/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (C) 2002-2004  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
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

#include <string.h>
#include <stdlib.h>

#include <glib-object.h>

#include "btypes.h"
#include "bparams.h"
#include "butils.h"


/**
 * b_filename_from_utf8:
 * @name: the filename in UTF-8 encoding
 * @root: an optional path to use
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Convert @name in UTF-8 encoding to a filename in the filesystem's
 * encoding. If @root is non-%NULL and @name is not an absolute
 * filename, the returned filename is built using @root as a prefix.
 *
 * Return value: a pointer to the newly allocated filename or %NULL in
 * case of an error. This value must be freed with g_free().
 **/
gchar *
b_filename_from_utf8 (const gchar  *name,
                      const gchar  *root,
                      GError      **error)
{
  gchar  *filename;
  GError *conv_error = NULL;

  g_return_val_if_fail (name != NULL, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, NULL);

  filename = g_filename_from_utf8 (name, -1, NULL, NULL, &conv_error);

  if (!filename)
    {
      const gchar *charset;

      g_get_charset (&charset);

      g_set_error (error, 0, 0,
                   "Couldn't convert filename '%s' to "
                   "your filesystem encoding (%s): %s",
                   name, charset, conv_error->message);
      g_error_free (conv_error);

      return NULL;
    }

  if (root && !g_path_is_absolute (filename))
    {
      gchar *tmp = g_build_filename (root, filename, NULL);
      g_free (filename);
      filename = tmp;
    }

  return filename;
}

/**
 * b_parse_int:
 * @str: the string to parse
 * @value: location to store the integer value
 *
 * Parse an integer value from a string.
 *
 * Return value: %TRUE if the string could be parsed, %FALSE otherwise
 **/
gboolean
b_parse_int (const gchar *str,
             gint        *value)
{
  gchar *err;
  glong  l;

  g_return_val_if_fail (str != NULL, FALSE);

  l = strtol (str, &err, 0);

  if (*str && *err)
    return FALSE;

  if (value)
    *value = l;

  return TRUE;
}

/**
 * b_parse_boolean:
 * @str: the string to parse
 * @value: location to store the boolean value
 *
 * Parse a boolean value from a string.
 *
 * Return value: %TRUE if the string could be parsed, %FALSE otherwise
 **/
gboolean
b_parse_boolean (const gchar *str,
                 gboolean    *value)
{
  g_return_val_if_fail (str != NULL, FALSE);

  if (g_ascii_strcasecmp (str, "yes") == 0)
    *value = TRUE;
  else if (g_ascii_strcasecmp (str, "no") == 0)
    *value = FALSE;
  else if (g_ascii_strcasecmp (str, "oui") == 0)
    *value = TRUE;
  else if (g_ascii_strcasecmp (str, "non") == 0)
    *value = FALSE;
  else
    return FALSE;

  return TRUE;
}

/**
 * b_parse_double:
 * @str: the string to parse
 * @value: location to store the double value
 *
 * Parse a double (floating-point) value from a string.
 *
 * Return value: %TRUE if the string could be parsed, %FALSE otherwise
 **/
gboolean
b_parse_double (const gchar *str,
                gdouble     *value)
{
  gchar  *end;
  gdouble d;

  g_return_val_if_fail (str != NULL, FALSE);

  d = g_ascii_strtod (str, &end);

  if (end && *end)
    return FALSE;

  if (value)
    *value = d;

  return TRUE;
}

/**
 * b_parse_coordinates:
 * @names: a %NULL-terminated array of names
 * @values: a %NULL-terminated array of values
 * @x: location to store the value of the x coordinate
 * @y: location to store the value of the y coordinate
 *
 * Parses a pair of name/value arrays looking for the names "x" and "y"
 * and tries to parse the associated values into integer values.
 *
 * Return value: %TRUE if both coordinates could be parsed, %FALSE otherwise
 **/
gboolean
b_parse_coordinates (const gchar **names,
                     const gchar **values,
                     gint         *x,
                     gint         *y)
{
  const gchar **name;
  const gchar **value;
  guint         f = 0;

  g_return_val_if_fail (names != NULL && values != NULL, FALSE);

  for (name = names, value = values; *name && *value; name++, value++)
    {
      if (!(f&1) && strcmp (*name, "x") == 0 && b_parse_int (*value, x))
        f |= 1;
      if (!(f&2) && strcmp (*name, "y") == 0 && b_parse_int (*value, y))
        f |= 2;
    }

  return (f == (1 | 2));
}

/**
 * b_parse_rectangle:
 * @names: a %NULL-terminated array of names
 * @values: a %NULL-terminated array of values
 * @rect: pointer to a #BRectangle to store the result
 *
 * Parses a pair of name/value arrays looking for the names "x", "y",
 * "width" and "heigth" and tries to parse the associated values into
 * a #BRectangle.
 *
 * Return value: %TRUE if the rectangle could be parsed, %FALSE otherwise
 **/
gboolean
b_parse_rectangle (const gchar **names,
                   const gchar **values,
                   BRectangle   *rect)
{
  const gchar **name;
  const gchar **value;
  guint         f = 0;

  g_return_val_if_fail (names != NULL && values != NULL, FALSE);
  g_return_val_if_fail (rect != NULL, FALSE);

  for (name = names, value = values; *name && *value; name++, value++)
    {
      if (!(f&1) && strcmp (*name, "x") == 0 &&
          b_parse_int (*value, &rect->x))
        f |= 1;

      if (!(f&2) && strcmp (*name, "y") == 0 &&
          b_parse_int (*value, &rect->y))
        f |= 2;

      if (!(f&4) && strcmp (*name, "width") == 0 &&
          b_parse_int (*value, &rect->w))
        f |= 4;

      if (!(f&8) && strcmp (*name, "height") == 0 &&
          b_parse_int (*value, &rect->h))
        f |= 8;
    }

  return (f == (1 | 2 | 4 | 8));
}

/**
 * b_parse_color:
 * @names: a %NULL-terminated array of names
 * @values: a %NULL-terminated array of values
 * @color: pointer to a #BColor to store the result
 *
 * Parses a pair of name/value arrays looking for the name "color" and
 * tries to convert the associated value to a #BColor. The color value
 * is expected to be in hexadecimal notation as in HTML.
 *
 * Return value: %TRUE if the color could be parsed, %FALSE otherwise
 **/
gboolean
b_parse_color (const gchar **names,
               const gchar **values,
               BColor       *color)
{
  const gchar **name;
  const gchar **value;

  g_return_val_if_fail (names != NULL && values != NULL, FALSE);
  g_return_val_if_fail (color != NULL, FALSE);

  for (name = names, value = values; *name && *value; name++, value++)
    {
      if (strcmp (*name, "color") == 0 && **value == '#')
        {
          gchar   *err;
          guint32  argb = strtoul (*value + 1, &err, 16);

          if (*err)
            continue;

          color->b = argb & 0xFF;  argb >>= 8;
          color->g = argb & 0xFF;  argb >>= 8;
          color->r = argb & 0xFF;  argb >>= 8;

          if (strlen (*value + 1) > 6)
            color->a = argb & 0xFF;
          else
            color->a = 0xFF;

          return TRUE;
        }
    }

  return FALSE;
}

/**
 * b_parse_param:
 * @object: a #GObject
 * @root: an optional string to use as root if a filename is set
 * @names: a %NULL-terminated array of names
 * @values: a %NULL-terminated array of values
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Parses a pair of name/value arrays looking for the names "key" and
 * "value". The key/value pair is then used to set the respective object
 * property by calling b_object_set_property().
 *
 * Return value: %TRUE if parsing was successful, %FALSE otherwise
 **/
gboolean
b_parse_param (GObject      *object,
               const gchar  *root,
               const gchar **names,
               const gchar **values,
               GError      **error)
{
  const gchar  *key = NULL;
  const gchar  *val = NULL;
  gint          i;

  g_return_val_if_fail (G_IS_OBJECT (object), FALSE);
  g_return_val_if_fail (names != NULL && values != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  for (i = 0; names[i] && values[i]; i++)
    {
      if (! strcmp (names[i], "key"))
        {
          key = values[i];
          val = NULL;
        }
      else if (! strcmp (names[i], "value"))
        {
          if (key)
            val = values[i];
        }
    }

  if (!key)
    {
      g_set_error (error, 0, 0,
                   "key attribute is missing for param element");
      return FALSE;
    }
  if (!val)
    {
      g_set_error (error, 0, 0,
                   "value attribute is missing for param element");
      return FALSE;
    }

  return b_object_set_property (object, key, val, root, error);
}

/**
 * b_object_set_property:
 * @object: a #GObject
 * @key: the name of the property
 * @value: the property value as a string
 * @root: an optional string to use as root if a filename property is set
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Sets the object property %key by interpreting the string
 * @value. This function takes care of converting the string to the
 * proper type. If the property is a %B_TYPE_FILENAME the filename is
 * build using the @root parameter.
 *
 * Return value: %TRUE on success, %FALSE otherwise
 **/
gboolean
b_object_set_property (GObject      *object,
                       const gchar  *key,
                       const gchar  *value,
                       const gchar  *root,
                       GError      **error)
{
  GObjectClass *class;
  GParamSpec   *spec;
  GValue        gvalue = { 0 };

  g_return_val_if_fail (G_IS_OBJECT (object), FALSE);
  g_return_val_if_fail (key != NULL, FALSE);
  g_return_val_if_fail (value != NULL, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  class = G_OBJECT_GET_CLASS (object);
  spec = g_object_class_find_property (class, key);

  if (!spec)
    {
      g_set_error (error, 0, 0,
                   "%s doesn't have a property named '%s'",
                   G_OBJECT_CLASS_NAME (class), key);
      return FALSE;
    }

  if (B_IS_PARAM_SPEC_FILENAME (spec))
    {
      gchar *filename = b_filename_from_utf8 (value, root, error);

      if (!filename)
        return FALSE;

      g_value_init (&gvalue, G_TYPE_STRING);
      g_value_take_string (&gvalue, filename);
    }
  else
    {
      g_value_init (&gvalue, G_TYPE_STRING);
      g_value_set_static_string (&gvalue, value);
    }

  g_object_set_property (object, key, &gvalue);

  g_value_unset (&gvalue);

  return TRUE;
}


void
b_rectangle_union (const BRectangle *src1,
                   const BRectangle *src2,
                   BRectangle       *dest)
{
  gint dest_x, dest_y;

  g_return_if_fail (src1 != NULL);
  g_return_if_fail (src2 != NULL);
  g_return_if_fail (dest != NULL);

  if (src1->w > 0 && src1->h > 0 && src2->w > 0 && src2->h > 0)
    {
      dest_x = MIN (src1->x, src2->x);
      dest_y = MIN (src1->y, src2->y);

      dest->w = MAX (src1->x + src1->w, src2->x + src2->w) - dest_x;
      dest->h = MAX (src1->y + src1->h, src2->y + src2->h) - dest_y;
      dest->x = dest_x;
      dest->y = dest_y;
    }
  else if (src1->w > 0 && src1->h > 0)
    {
      *dest = *src1;
    }
  else if (src2->w > 0 && src2->h > 0)
    {
      *dest = *src2;
    }
  else
    {
      dest->x = dest->y = dest->w = dest->h = 0;
    }
}

gboolean
b_rectangle_intersect (const BRectangle *src1,
                       const BRectangle *src2,
                       BRectangle       *dest)
{
  gint dest_x, dest_y;
  gint dest_w, dest_h;

  g_return_val_if_fail (src1 != NULL, FALSE);
  g_return_val_if_fail (src2 != NULL, FALSE);

  dest_x = MAX (src1->x, src2->x);
  dest_y = MAX (src1->y, src2->y);
  dest_w = MIN (src1->x + src1->w, src2->x + src2->w) - dest_x;
  dest_h = MIN (src1->y + src1->h, src2->y + src2->h) - dest_y;

  if (dest_w > 0 && dest_h > 0)
    {
      if (dest)
        {
          dest->x = dest_x;
          dest->y = dest_y;
          dest->w = dest_w;
          dest->h = dest_h;
        }

      return TRUE;
    }
  else if (dest)
    {
      dest->w = 0;
      dest->h = 0;
    }

  return FALSE;
}

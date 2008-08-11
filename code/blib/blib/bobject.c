/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
 *                          Michael Natterer <mitch@gimp.org>
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
#include "bobject.h"

enum
{
  PROP_0,
  PROP_FILENAME,
  PROP_NAME
};


static void  b_object_class_init   (BObjectClass *class);
static void  b_object_init         (BObject      *object);
static void  b_object_finalize     (GObject      *object);
static void  b_object_set_property (GObject      *object,
                                    guint         property_id,
                                    const GValue *value,
                                    GParamSpec   *pspec);
static void  b_object_get_property (GObject      *object,
                                    guint         property_id,
                                    GValue       *value,
                                    GParamSpec   *pspec);


static GObjectClass *parent_class = NULL;


GType
b_object_get_type (void)
{
  static GType object_type = 0;

  if (!object_type)
    {
      static const GTypeInfo object_info =
      {
        sizeof (BObjectClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_object_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BObject),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_object_init,
      };

      object_type = g_type_register_static (G_TYPE_OBJECT,
                                           "BObject", &object_info, 0);
    }
  
  return object_type;
}

static void
b_object_class_init (BObjectClass *class)
{
  GObjectClass *object_class;
  GParamSpec   *param_spec;

  parent_class = g_type_class_peek_parent (class);
  object_class = G_OBJECT_CLASS (class);

  object_class->set_property = b_object_set_property;
  object_class->get_property = b_object_get_property;
  object_class->finalize     = b_object_finalize;

  param_spec = g_param_spec_string ("filename", NULL,
				    "The filename associated with the object. "
				    "This is a string in the filesystems's "
                                    "encoding.",
				    NULL,
                                    G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_FILENAME, param_spec);

  param_spec = g_param_spec_string ("name", NULL,
				    "The name associated with the object. "
				    "This is a string in UTF-8 encoding.",
				    NULL,
                                    G_PARAM_READWRITE);
  g_object_class_install_property (object_class, PROP_NAME, param_spec);
}

static void
b_object_init (BObject *object)
{
  object->filename = NULL;
  object->name     = NULL;
}

static void
b_object_finalize (GObject *object)
{
  BObject *bobject;

  bobject = B_OBJECT (object);

  g_free (bobject->filename);
  g_free (bobject->name);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
b_object_set_property (GObject      *object,
                       guint         property_id,
                       const GValue *value,
                       GParamSpec   *pspec)
{
  switch (property_id)
    {
    case PROP_FILENAME:
      b_object_set_filename (B_OBJECT (object), g_value_get_string (value));
      break;
    case PROP_NAME:
      b_object_set_name (B_OBJECT (object), g_value_get_string (value));
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

static void
b_object_get_property (GObject     *object,
                       guint        property_id,
                       GValue      *value,
                       GParamSpec  *pspec)
{
  switch (property_id)
    {
    case PROP_FILENAME:
      g_value_set_string (value, B_OBJECT (object)->filename);
      break;
    case PROP_NAME:
      g_value_set_string (value, B_OBJECT (object)->name);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}

/**
 * b_object_set_filename:
 * @object: a #BObject.
 * @filename: the new filename, a nul-terminated string in the filesystem's
 * encoding.
 * 
 * Sets the filename associated with @object. 
 **/
void
b_object_set_filename (BObject     *object,
                       const gchar *filename)
{
  g_return_if_fail (B_IS_OBJECT (object));

  g_free (object->filename);
  object->filename = g_strdup (filename);

  g_object_notify (G_OBJECT (object), "filename");
}

/**
 * b_object_set_name:
 * @object: a #BObject.
 * @name: the new name, a nul-terminated string in UTF-8 encoding.
 * 
 * Sets the name associated with @object. 
 **/
void
b_object_set_name (BObject    *object,
                  const gchar *name)
{
  g_return_if_fail (B_IS_OBJECT (object));
  g_return_if_fail (g_utf8_validate (name, -1, NULL));

  g_free (object->name);
  object->name = g_strdup (name);

  g_object_notify (G_OBJECT (object), "name");
}

/**
 * b_object_get_filename:
 * @object: a #BObject. 
 * 
 * This functions retrieves the filename associated with @object. The
 * returned value must not be freed.
 * 
 * Return value: the fileanme or %NULL if no filename was set for @object.
 **/
const gchar *
b_object_get_filename (BObject *object)
{
  g_return_val_if_fail (B_IS_OBJECT (object), NULL);

  return object->filename;
}

/**
 * b_object_get_name:
 * @object: a #BObject. 
 * 
 * This functions retrieves the name associated with @object. If no
 * name has been set for @object, the name is generated from the
 * @object's filename. The returned value must not be freed.
 * 
 * Return value: the name or %NULL if no name and no filename was set
 * for @object.
 **/
const gchar *
b_object_get_name (BObject *object)
{
  g_return_val_if_fail (B_IS_OBJECT (object), NULL);

  if (!object->name && object->filename)
    object->name = g_filename_to_utf8 (object->filename, -1, NULL, NULL, NULL);

  return object->name;
}

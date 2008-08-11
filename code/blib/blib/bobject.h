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

#ifndef __B_OBJECT_H__
#define __B_OBJECT_H__

G_BEGIN_DECLS

#define B_TYPE_OBJECT            (b_object_get_type ())
#define B_OBJECT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_OBJECT, BObject))
#define B_OBJECT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_OBJECT, BObjectClass))
#define B_IS_OBJECT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_OBJECT))
#define B_IS_OBJECT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_OBJECT))
#define B_OBJECT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), B_TYPE_OBJECT, BObjectClass))


typedef struct _BObjectClass BObjectClass;

struct _BObjectClass
{
  GObjectClass  parent_class;
};

struct _BObject
{
  GObject       parent_instance;
  
  gchar        *filename;   /* in filesystem encoding */
  gchar        *name;       /* UTF-8 encoded, defaults to converted filename */
};


GType         b_object_get_type     (void) G_GNUC_CONST;

void          b_object_set_filename (BObject      *object,
                                     const gchar  *filename);
void          b_object_set_name     (BObject      *object,
                                     const gchar  *name);

const gchar * b_object_get_filename (BObject      *object);
const gchar * b_object_get_name     (BObject      *object);

G_END_DECLS

#endif /* __B_OBJECT_H__ */

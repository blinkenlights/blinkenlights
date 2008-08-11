/* blccc - Blinkenlights Chaos Control Center
 *
 * Copyright (c) 2001-2003  Sven Neumann <sven@gimp.org>
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

#ifndef __BL_DISPATCH_H__
#define __BL_DISPATCH_H__

#define MAX_DISPATCH_CHARS 8

G_BEGIN_DECLS

#define BL_TYPE_DISPATCH            (bl_dispatch_get_type ())
#define BL_DISPATCH(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_DISPATCH, BlDispatch))
#define BL_DISPATCH_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_DISPATCH, BlDispatchClass))
#define BL_IS_DISPATCH(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_DISPATCH))
#define BL_IS_DISPATCH_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BL_TYPE_DISPATCH))
#define BL_DISPATCH_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), BL_TYPE_DISPATCH, BlDispatchClass))

typedef struct _BlDispatchClass BlDispatchClass;

struct _BlDispatch
{
  BModule         parent_instance;

  gchar          *filename;
  BlLovemodules  *lovemodules;

  gint            chars;
  gchar           code[MAX_DISPATCH_CHARS + 1];

  BModule        *module;
  GList          *devices;
};

struct _BlDispatchClass
{
  BModuleClass    parent_class;
};

GType bl_dispatch_get_type (void) G_GNUC_CONST;

G_END_DECLS

#endif /* __BL_DISPATCH_H__ */

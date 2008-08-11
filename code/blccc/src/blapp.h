/* blccc - Blinkenlights Chaos Control Center
 *
 * Copyright (c) 2001-2002  Sven Neumann <sven@gimp.org>
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

#ifndef __BL_APP_H__
#define __BL_APP_H__

#include "blplaylistitem.h"

G_BEGIN_DECLS

#define BL_TYPE_APP         (bl_app_get_type ())
#define BL_APP(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_APP, BlApp))
#define BL_APP_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_APP, BlAppClass))
#define BL_IS_APP(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_APP))

typedef struct _BlAppClass  BlAppClass;

struct _BlApp
{
  BlPlaylistItem  parent_instance;

  gchar          *name;
  gchar          *number;
  gchar          *sound;
  gchar          *sound_loop;
  gboolean        public;
  gboolean        disabled;
  gint            priority;

  GList          *lines;
};

struct _BlAppClass
{
  BlPlaylistItemClass  parent_class;
};

GType   bl_app_get_type            (void) G_GNUC_CONST;
BlApp * bl_app_new_from_attributes (const gchar  **names,
                                    const gchar  **values,
                                    const gchar   *root,
                                    GError       **error);

G_END_DECLS

#endif /* __BL_APP_H__ */

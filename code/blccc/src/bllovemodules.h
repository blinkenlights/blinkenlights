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

#ifndef __BL_LOVEMODULES_H__
#define __BL_LOVEMODULES_H__

G_BEGIN_DECLS

#include "blloveletters.h"

#define BL_TYPE_LOVEMODULES         (bl_lovemodules_get_type ())
#define BL_LOVEMODULES(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_LOVEMODULES, BlLovemodules))
#define BL_LOVEMODULES_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_LOVEMODULES, BlLovemodulesClass))
#define BL_IS_LOVEMODULES(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_LOVEMODULES))


typedef struct _BlLovemodulesClass  BlLovemodulesClass;

struct _BlLovemodulesClass
{
  BlLovelettersClass   parent_class;
};

struct _BlLovemodules
{
  BlLoveletters        parent_instance;

  GHashTable          *hash;
};


GType           bl_lovemodules_get_type      (void) G_GNUC_CONST;

BlLovemodules * bl_lovemodules_new_from_file (const gchar    *filename,
                                              GError        **error);
gboolean        bl_lovemodules_parse         (BlLovemodules  *lovemodules,
                                              GError        **error);
const gchar   * bl_lovemodules_lookup        (BlLovemodules  *lovemodules,
                                              const gchar    *id);

G_END_DECLS

#endif /* __BL_LOVEMODULES_H__ */

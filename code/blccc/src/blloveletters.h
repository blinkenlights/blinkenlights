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

#ifndef __BL_LOVELETTERS_H__
#define __BL_LOVELETTERS_H__

G_BEGIN_DECLS

#define BL_TYPE_LOVELETTERS         (bl_loveletters_get_type ())
#define BL_LOVELETTERS(obj)         (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_LOVELETTERS, BlLoveletters))
#define BL_LOVELETTERS_CLASS(klass) (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_LOVELETTERS, BlLovelettersClass))
#define BL_IS_LOVELETTERS(obj)      (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_LOVELETTERS))


typedef struct _BlLovelettersClass  BlLovelettersClass;

struct _BlLovelettersClass
{
  BObjectClass   parent_class;
};

struct _BlLoveletters
{
  BObject        parent_instance;

  GHashTable    *hash;
};


GType           bl_loveletters_get_type      (void) G_GNUC_CONST;

BlLoveletters * bl_loveletters_new_from_file (const gchar    *filename,
                                              GError        **error);
gboolean        bl_loveletters_parse         (BlLoveletters  *loveletters,
                                              GError        **error);
const gchar   * bl_loveletters_lookup        (BlLoveletters  *loveletters,
                                              const gchar    *id);

void            bl_loveletter_validate       (const gchar    *id,
                                              const gchar    *vanity);


G_END_DECLS

#endif /* __BL_LOVELETTERS_H__ */

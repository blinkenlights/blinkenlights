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

#ifndef __BL_CCC_H__
#define __BL_CCC_H__

G_BEGIN_DECLS

#define BL_TYPE_CCC            (bl_ccc_get_type ())
#define BL_CCC(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_CCC, BlCcc))
#define BL_CCC_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_CCC, BlCccClass))
#define BL_IS_CCC(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_CCC))
#define BL_IS_CCC_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BL_TYPE_CCC))

typedef struct _BlCccClass  BlCccClass;

struct _BlCccClass
{
  GObjectClass   parent_class;
};

struct _BlCcc
{
  GObject        parent_instance;

  BlConfig      *config;
  
  GMutex        *mutex;
  BlTheater     *theater;
  BlPlaylist    *playlist;
  BEffects      *effects;
  BlIsdn        *isdn;

  BlApp         *active_app;
};

GType       bl_ccc_get_type     (void) G_GNUC_CONST;
BlCcc     * bl_ccc_new          (BlConfig     *config);

gchar     * bl_ccc_status       (BlCcc        *ccc);
gboolean    bl_ccc_reload       (BlCcc        *ccc);
gboolean    bl_ccc_load         (BlCcc        *ccc,
                                 const gchar  *filename,
                                 gboolean      instant_change);
gchar     * bl_ccc_list         (BlCcc        *ccc);
gchar     * bl_ccc_next         (BlCcc        *ccc);
void        bl_ccc_kill         (BlCcc        *ccc);

void        bl_ccc_event        (BlCcc        *ccc,
                                 BModuleEvent *event);

gboolean    bl_ccc_add          (BlCcc        *ccc,
                                 const gchar  *host,
                                 GError      **error);
gboolean    bl_ccc_remove       (BlCcc        *ccc,
                                 const gchar  *host,
                                 GError      **error);

gboolean    bl_ccc_app_enable   (BlCcc        *ccc,
                                 const gchar  *number);
gboolean    bl_ccc_app_disable  (BlCcc        *ccc,
                                 const gchar  *number);

void        bl_ccc_isdn_block   (BlCcc       *ccc);
void        bl_ccc_isdn_unblock (BlCcc       *ccc);

G_END_DECLS

#endif /* __BL_CCC_H__ */

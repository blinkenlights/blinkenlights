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

#ifndef __BL_LISTEN_H__
#define __BL_LISTEN_H__

G_BEGIN_DECLS

#define BL_TYPE_LISTEN            (bl_listen_get_type ())
#define BL_LISTEN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_LISTEN, BlListen))
#define BL_LISTEN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_LISTEN, BlListenClass))
#define BL_IS_LISTEN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_LISTEN))
#define BL_IS_LISTEN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BL_TYPE_LISTEN))

typedef struct _BlListenClass  BlListenClass;

struct _BlListenClass
{
  GObjectClass  parent_class;
};

struct _BlListen
{
  GObject       parent_instance;

  gint          sock;
  BlCcc        *ccc;
};

GType      bl_listen_get_type (void) G_GNUC_CONST;
BlListen * bl_listen_new      (gint   port,
                               BlCcc *ccc);

G_END_DECLS

#endif /* __BL_LISTEN_H__ */

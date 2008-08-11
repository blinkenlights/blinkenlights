/* blccc - BlinkenLigths Chaos Control Center
 *
 * Copyright (c) 2001  Sven Neumann <sven@gimp.org>
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

#ifndef __BL_ISDN_H__
#define __BL_ISDN_H__

#include <sys/types.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#define BL_TYPE_ISDN            (bl_isdn_get_type ())
#define BL_ISDN(obj)            (GTK_CHECK_CAST ((obj), BL_TYPE_ISDN, BlIsdn))
#define BL_ISDN_CLASS(klass)    (GTK_CHECK_CLASS_CAST ((klass), BL_TYPE_ISDN, BlIsdnClass))
#define BL_IS_ISDN(obj)         (GTK_CHECK_TYPE ((obj), BL_TYPE_ISDN))
#define BL_IS_ISDN_CLASS(klass) (GTK_CHECK_CLASS_TYPE ((klass), BL_TYPE_ISDN))

typedef struct _BlIsdnClass  BlIsdnClass;

struct _BlIsdnClass
{
  GtkObjectClass  parent_class;

  void (* state_changed)  (BlIsdn    *isdn);
  void (* dial_tone)      (BlIsdn    *isdn,
                           gint       tone0,
                           gint       tone1);
};

struct _BlIsdn
{
  GtkObject       parent_instance;

  gint            sock;
  
  gboolean        sock_initialized;
  gboolean        state_initialized;
  gboolean        packet_pending;
  gint            no_packets;

  gboolean        line0_offhook;
  gboolean        line1_offhook;
};

GtkType    bl_isdn_get_type (void);
BlIsdn   * bl_isdn_new      (gint udp_port);


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __BL_ISDN_H__ */

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

#ifndef __BL_ISDN_H__
#define __BL_ISDN_H__

G_BEGIN_DECLS


typedef enum
{
  BL_ISDN_NO_REASON                   = 0,
  BL_ISDN_REASON_NORMAL_CALL_CLEARING = 16,
  BL_ISDN_REASON_USER_BUSY            = 17,
  BL_ISDN_REASON_NO_USER_RESPONSE     = 18
} BlIsdnReason;


#define BL_TYPE_ISDN            (bl_isdn_get_type ())
#define BL_ISDN(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), BL_TYPE_ISDN, BlIsdn))
#define BL_ISDN_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), BL_TYPE_ISDN, BlIsdnClass))
#define BL_IS_ISDN(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), BL_TYPE_ISDN))
#define BL_IS_ISDN_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), BL_TYPE_ISDN))

typedef struct _BlIsdnClass  BlIsdnClass;

struct _BlIsdnClass
{
  GObjectClass  parent_class;

  void (* incoming_call)  (BlIsdn     *isdn,
                           BlIsdnLine *line);
  void (* state_changed)  (BlIsdn     *isdn,
                           BlIsdnLine *line);
  void (* key_pressed)    (BlIsdn     *isdn,
                           BlIsdnLine *line,
                           BModuleKey  key);
};

struct _BlIsdn
{
  GObject       parent_instance;

  BlConfig     *config;

  GSource      *source;

  gint          listen_sock;
  gint          send_sock;

  guint         heartbeat;

  gboolean      sock_initialized;
  gboolean      state_initialized;
  gboolean      packet_pending;
  gint          no_packets;

  gboolean      blocked;
  gint          num_lines;
  BlIsdnLine   *lines;
};

struct _BlIsdnLine
{
  gint          channel;
  gboolean      offhook;
  gchar        *called_number;
  gchar        *calling_number;

  /*  used only by BlCcc  */
  BlApp        *app;
};


GType    bl_isdn_get_type    (void) G_GNUC_CONST;
BlIsdn * bl_isdn_new         (BlConfig     *config,
                              GError      **error);
void     bl_isdn_call_accept (BlIsdn       *isdn,
                              BlIsdnLine   *line,
                              const gchar  *sound,
                              const gchar  *sound_loop);
void     bl_isdn_call_hangup (BlIsdn       *isdn,
                              BlIsdnLine   *line,
                              BlIsdnReason  reason);
void     bl_isdn_block       (BlIsdn       *isdn);
void     bl_isdn_unblock     (BlIsdn       *isdn);


G_END_DECLS

#endif /* __BL_ISDN_H__ */

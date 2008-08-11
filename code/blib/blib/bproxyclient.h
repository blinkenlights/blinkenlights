/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2003  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
 *                          Michael Natterer <mitch@gimp.org>
 *	                    Daniel Mack <daniel@yoobay.net>
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


#ifndef __B_PROXY_CLIENT_H__
#define __B_PROXY_CLIENT_H__

G_BEGIN_DECLS

#include <blib/breceiver.h>

#define B_TYPE_PROXY_CLIENT            (b_proxy_client_get_type ())
#define B_PROXY_CLIENT(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_PROXY_CLIENT, BProxyClient))
#define B_PROXY_CLIENT_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_PROXY_CLIENT, BProxyClientClass))
#define B_IS_PROXY_CLIENT(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_PROXY_CLIENT))
#define B_IS_PROXY_CLIENT_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_PROXY_CLIENT))
#define B_PROXY_CLIENT_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), B_TYPE_PROXY_CLIENT, BProxyClientClass))


typedef struct _BProxyClientClass BProxyClientClass;

struct _BProxyClientClass
{
  BReceiverClass     parent_class;
};

struct _BProxyClient
{
  BReceiver          parent_instance;

  gint               fd;
};

GType          b_proxy_client_get_type       (void) G_GNUC_CONST;
BProxyClient * b_proxy_client_new            (const gchar       *proxy_host,
                                              gint               proxy_port,
                                              gint               listen_port,
                                              BReceiverCallback  callback,
                                              gpointer           callback_data,
                                              GError           **error);
gboolean       b_proxy_client_send_heartbeat (BProxyClient      *client);

G_END_DECLS

#endif /* __B_PROXY_CLIENT_H__ */

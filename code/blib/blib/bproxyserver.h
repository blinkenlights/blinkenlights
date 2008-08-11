/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2004  The Blinkenlights Crew
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


#ifndef __B_PROXY_SERVER_H__
#define __B_PROXY_SERVER_H__

G_BEGIN_DECLS

#include <blib/breceiver.h>

#define B_TYPE_PROXY_SERVER            (b_proxy_server_get_type ())
#define B_PROXY_SERVER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_PROXY_SERVER, BProxyServer))
#define B_PROXY_SERVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_PROXY_SERVER, BProxyServerClass))
#define B_IS_PROXY_SERVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_PROXY_SERVER))
#define B_IS_PROXY_SERVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_PROXY_SERVER))
#define B_PROXY_SERVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), B_TYPE_PROXY_SERVER, BProxyServerClass))


typedef struct _BProxyServerClass BProxyServerClass;

struct _BProxyServerClass
{
  BReceiverClass     parent_class;

  void  (* client_added)   (BProxyServer *server,
                            const gchar  *host,
                            gint          port);
  void  (* client_removed) (BProxyServer *server,
                            const gchar  *host,
                            gint          port);
};

struct _BProxyServer
{
  BReceiver          parent_instance;

  gint               fd;
  GHashTable        *clients;
};


GType          b_proxy_server_get_type    (void) G_GNUC_CONST;
BProxyServer * b_proxy_server_new         (gint           port,
                                           GError       **error);

gboolean       b_proxy_server_send_packet (BProxyServer  *server,
                                           BPacket       *packet);
gint           b_proxy_server_num_clients (BProxyServer  *server);

G_END_DECLS

#endif /* __B_PROXY_SERVER_H__ */

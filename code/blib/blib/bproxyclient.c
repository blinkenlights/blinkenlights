/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2003  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
 *                          Michael Natterer <mitch@gimp.org>
 *                          Daniel Mack <daniel@yoobay.net>
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

#include "config.h"

#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib-object.h>

#ifdef G_OS_WIN32
#include <winsock2.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "btypes.h"
#include "bpacket.h"
#include "bproxyclient.h"
#include "breceiver.h"
#include "bsocket.h"


static void   b_proxy_client_class_init (BProxyClientClass *class);
static void   b_proxy_client_init       (BProxyClient      *client);
static void   b_proxy_client_finalize   (GObject           *object);


static GObjectClass *parent_class = NULL;


GType
b_proxy_client_get_type (void)
{
  static GType client_type = 0;

  if (!client_type)
    {
      static const GTypeInfo client_info =
      {
        sizeof (BProxyClientClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_proxy_client_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BProxyClient),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_proxy_client_init,
      };

      client_type = g_type_register_static (B_TYPE_RECEIVER,
                                            "BProxyClient", &client_info, 0);
    }

  return client_type;
}

static void
b_proxy_client_class_init (BProxyClientClass *class)
{
  GObjectClass *object_class;

  parent_class = g_type_class_peek_parent (class);
  object_class = G_OBJECT_CLASS (class);

  object_class->finalize     = b_proxy_client_finalize;
}

static void
b_proxy_client_init (BProxyClient *client)
{
  client->fd = -1;
}

static void
b_proxy_client_finalize (GObject *object)
{
  BProxyClient *client = B_PROXY_CLIENT (object);

  if (client->fd != -1)
    {
      close (client->fd);
      client->fd = -1;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * b_proxy_client_new:
 * @proxy_host: the blinkenproxy host
 * @proxy_port: the blinkenproxy port
 * @listen_port: the local port to listen to for Blinkenlights packages
 *               or -1 to bind to an arbitrary free local port
 * @callback: the function to call when a new frame arrives
 * @callback_data: data to pass to the @callback
 * @error: return location for a possible error
 *
 * Creates a new #BProxyClient object, ready to use.
 *
 * Return value: a newly allocate #BProxyClient object
 **/
BProxyClient *
b_proxy_client_new (const gchar        *proxy_host,
                    gint                proxy_port,
                    gint                listen_port,
                    BReceiverCallback   callback,
                    gpointer            callback_data,
                    GError            **error)
{
  BProxyClient       *client;
  struct hostent     *dest;
  struct sockaddr_in  sock;
  gint                fd;

  g_return_val_if_fail (proxy_host != NULL && proxy_host != '\0', NULL);
  g_return_val_if_fail (proxy_port > 0, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! b_socket_init (error))
    return NULL;

  dest = gethostbyname (proxy_host);
  if (dest == NULL)
    {
      g_set_error (error, 0, 0, "Unable to resolve host '%s'", proxy_host);
      return NULL;
    }

  fd = b_socket_udp_new (B_SO_REUSEADDR, error);
  if (fd < 0)
    return NULL;

  if (listen_port > 0)
    {
      sock.sin_addr.s_addr = INADDR_ANY;
      sock.sin_family      = AF_INET;
      sock.sin_port        = g_htons (listen_port);

      if (bind (fd, (struct sockaddr *) &sock, sizeof (sock)) < 0)
        {
          g_set_error (error, 0, 0,
                       "Can't bind socket for %s to local port %d: %s\n",
                       proxy_host, listen_port, g_strerror (errno));
          close (fd);
          return NULL;
        }
    }

  sock.sin_family = AF_INET;
  sock.sin_port   = g_htons (proxy_port);
  memcpy (&sock.sin_addr.s_addr, dest->h_addr_list[0], dest->h_length);

  if (connect (fd, (struct sockaddr *) &sock, sizeof (sock)) < 0)
   {
     g_set_error (error, 0, 0, "Can't connect socket to %s:%d: %s\n",
                  proxy_host, proxy_port, g_strerror (errno));
     close (fd);
     return NULL;
   }

  client = g_object_new (B_TYPE_PROXY_CLIENT,
                         "callback",      callback,
                         "callback_data", callback_data,
                         NULL);

  client->fd = fd;

  b_receiver_listen_fd (B_RECEIVER (client), fd);

  return client;
}

/**
 * b_proxy_client_send_heartbeat:
 * @client: a #BProxyClient
 *
 * Make the @client send a heartbeat packet. This function should
 * be called periodically with the suggested interval of
 * %B_HEARTBEAT_INTERVAL. To achieve, use this function with
 * g_timeout_add().
 *
 * Return value: always %TRUE
 **/
gboolean
b_proxy_client_send_heartbeat (BProxyClient *client)
{
  BPacket packet;

  g_return_val_if_fail (B_IS_PROXY_CLIENT (client), FALSE);

  if (client->fd == -1)
    return TRUE;

  memset (&packet, 0, sizeof (BPacket));

  packet.header.heartbeat_h.magic = MAGIC_HEARTBEAT;

  b_packet_hton (&packet);

  send (client->fd, &packet, sizeof (BPacket), 0);

  return TRUE;
}

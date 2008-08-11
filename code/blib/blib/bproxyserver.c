/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2004  The Blinkenlights Crew
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
#include "bmarshal.h"
#include "bpacket.h"
#include "bproxyserver.h"
#include "breceiver.h"
#include "bsocket.h"


#define  HEARTBEAT_TIMEOUT  (B_HEARTBEAT_INTERVAL * 12)

enum
{
  CLIENT_ADDED,
  CLIENT_REMOVED,
  LAST_SIGNAL
};

typedef struct
{
  BProxyServer *server;
  gulong        addr;
  gushort       port;
  GSource      *timeout;
} BClient;


static void      b_proxy_server_class_init (BProxyServerClass *klass);
static void      b_proxy_server_init       (BProxyServer      *server);
static void      b_proxy_server_finalize   (GObject           *object);

static gboolean  b_proxy_server_heartbeat_callback (BProxyServer *server,
                                                    BPacket      *packet,
                                                    gpointer      data);

static guint     b_client_hash             (const BClient     *client);
static gboolean  b_client_equal            (const BClient     *a,
                                            const BClient     *b);
static void      b_client_add              (const BClient     *template);
static gboolean  b_client_remove           (BClient           *client);

static void      b_client_send_packet      (gpointer           key,
                                            BClient           *client,
                                            BPacket           *packet);


static guint         server_signals[LAST_SIGNAL] = { 0 };
static GObjectClass *parent_class                = NULL;


GType
b_proxy_server_get_type (void)
{
  static GType server_type = 0;

  if (!server_type)
    {
      static const GTypeInfo server_info =
      {
        sizeof (BProxyServerClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_proxy_server_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BProxyServer),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_proxy_server_init,
      };

      server_type = g_type_register_static (B_TYPE_RECEIVER,
                                            "BProxyServer", &server_info, 0);
    }

  return server_type;
}

static void
b_proxy_server_class_init (BProxyServerClass *klass)
{
  GObjectClass *object_class;

  parent_class = g_type_class_peek_parent (klass);
  object_class = G_OBJECT_CLASS (klass);

  object_class->finalize = b_proxy_server_finalize;

  server_signals[CLIENT_ADDED] =
    g_signal_new ("client_added",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BProxyServerClass, client_added),
                  NULL, NULL,
                  b_marshal_VOID__STRING_INT,
                  G_TYPE_NONE,
                  2, G_TYPE_STRING, G_TYPE_INT);
  server_signals[CLIENT_REMOVED] =
    g_signal_new ("client_removed",
                  G_TYPE_FROM_CLASS (klass),
                  G_SIGNAL_RUN_FIRST,
                  G_STRUCT_OFFSET (BProxyServerClass, client_removed),
                  NULL, NULL,
                  b_marshal_VOID__STRING_INT,
                  G_TYPE_NONE,
                  2, G_TYPE_STRING, G_TYPE_INT);
}

static void
b_proxy_server_init (BProxyServer *server)
{
  server->fd = -1;
}

static void
b_proxy_server_finalize (GObject *object)
{
  BProxyServer *server = B_PROXY_SERVER (object);

  if (server->fd != -1)
    {
      close (server->fd);
      server->fd = -1;
    }

  if (server->clients)
    {
      g_hash_table_destroy (server->clients);
      server->clients = NULL;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * b_proxy_server_new:
 * @port:  the local port to listen to for heartbeat packages
 * @error: return location for a possible error
 *
 * Creates a new #BProxyServer object, ready to use.
 *
 * Return value: a newly allocate #BProxyServer object
 **/
BProxyServer *
b_proxy_server_new (gint     port,
                    GError **error)
{
  BProxyServer       *server;
  struct sockaddr_in  sock;
  gint                fd;

  g_return_val_if_fail (port > 0, NULL);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! b_socket_init (error))
    return NULL;

  fd = b_socket_udp_new (B_SO_REUSEADDR, error);
  if (fd < 0)
    return NULL;

  sock.sin_addr.s_addr = INADDR_ANY;
  sock.sin_family      = AF_INET;
  sock.sin_port        = g_htons (port);

  if (bind (fd, (struct sockaddr *) &sock, sizeof (sock)) < 0)
    {
      g_set_error (error, 0, 0,
                   "Can't bind socket to local port %d: %s\n",
                   port, g_strerror (errno));
      close (fd);
      return NULL;
    }

  server = g_object_new (B_TYPE_PROXY_SERVER,
                         "callback", b_proxy_server_heartbeat_callback,
                         NULL);

  server->fd = fd;

  server->clients = g_hash_table_new_full ((GHashFunc)  b_client_hash,
                                           (GEqualFunc) b_client_equal,
                                           (GDestroyNotify) NULL,
                                           (GDestroyNotify) g_free);

  b_receiver_listen_fd (B_RECEIVER (server), fd);

  return server;
}

gboolean
b_proxy_server_send_packet (BProxyServer *server,
                            BPacket      *packet)
{
  g_return_val_if_fail (B_IS_PROXY_SERVER (server), FALSE);
  g_return_val_if_fail (packet != NULL, FALSE);

  if (server->clients)
    g_hash_table_foreach (server->clients,
                          (GHFunc) b_client_send_packet, packet);

  return TRUE;
}

gint
b_proxy_server_num_clients (BProxyServer *server)
{
  g_return_val_if_fail (B_IS_PROXY_SERVER (server), 0);

  return (server->clients ? g_hash_table_size (server->clients) : 0);
}

static gboolean
b_proxy_server_heartbeat_callback (BProxyServer *server,
                                   BPacket      *packet,
                                   gpointer      data)
{
  BClient  lookup;
  BClient *client;

  if (packet->header.heartbeat_h.magic   != MAGIC_HEARTBEAT ||
      packet->header.heartbeat_h.version != 0)
    return TRUE;

  lookup.server = server;
  lookup.addr   = B_RECEIVER (server)->addr;
  lookup.port   = B_RECEIVER (server)->port;

  client = g_hash_table_lookup (server->clients, &lookup);

  if (client)
    {
      g_source_destroy (client->timeout);

      client->timeout = g_timeout_source_new (HEARTBEAT_TIMEOUT);
      g_source_set_callback (client->timeout,
                             (GSourceFunc) b_client_remove, client, NULL);
      g_source_attach (client->timeout, NULL);
    }
  else
    {
      b_client_add (&lookup);
    }

  return TRUE;
}

static guint
b_client_hash (const BClient *client)
{
  return (guint) client->addr;
}

static gboolean
b_client_equal (const BClient *a,
                const BClient *b)
{
  return (a->addr == b->addr && a->port == b->port);
}

static void
b_client_add (const BClient *template)
{
  BClient        *client = g_memdup (template, sizeof (BClient));
  struct in_addr  addr;
  const gchar    *host;
  gint            port;

  client->timeout = g_timeout_source_new (HEARTBEAT_TIMEOUT);
  g_source_set_callback (client->timeout,
                         (GSourceFunc) b_client_remove, client, NULL);
  g_source_attach (client->timeout, NULL);

  g_hash_table_insert (client->server->clients, client, client);

  addr.s_addr = client->addr;

  host = inet_ntoa (addr);
  port = g_ntohs (client->port);

  g_signal_emit (client->server,
                 server_signals[CLIENT_ADDED], 0,
                 host, port);
}

static gboolean
b_client_remove (BClient *client)
{
  BProxyServer   *server = client->server;
  struct in_addr  addr;
  const gchar    *host;
  gint            port;

  addr.s_addr = client->addr;

  host = inet_ntoa (addr);
  port = g_ntohs (client->port);

  g_hash_table_remove (server->clients, client);

  g_signal_emit (server,
                 server_signals[CLIENT_REMOVED], 0,
                 host, port);

  return FALSE;
}

static void
b_client_send_packet (gpointer  key,
                      BClient  *client,
                      BPacket  *packet)
{
  struct sockaddr_in  addr;
  gsize               size = b_packet_size (packet);
  BPacket            *copy = g_memdup (packet, size);

  b_packet_hton (copy);

  addr.sin_family      = AF_INET;
  addr.sin_port        = client->port;
  addr.sin_addr.s_addr = client->addr;

  sendto (client->server->fd, copy, size, 0,
          (struct sockaddr *) &addr, sizeof (addr));

  g_free (copy);
}


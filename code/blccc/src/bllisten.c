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

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>

#include <glib-object.h>

#include "bltypes.h"

#include "blaccept.h"
#include "bllisten.h"


static void  bl_listen_class_init (BlListenClass  *class);
static void  bl_listen_init       (BlListen       *listen);
static void  bl_listen_finalize   (GObject        *object);


static GObjectClass *parent_class = NULL;


GType
bl_listen_get_type (void)
{
  static GType listen_type = 0;

  if (!listen_type)
    {
      static const GTypeInfo listen_info =
      {
        sizeof (BlListenClass),
	(GBaseInitFunc) NULL,
	(GBaseFinalizeFunc) NULL,
	(GClassInitFunc) bl_listen_class_init,
	NULL,		/* class_finalize */
	NULL,		/* class_data     */
	sizeof (BlListen),
	0,              /* n_preallocs    */
	(GInstanceInitFunc) bl_listen_init,
      };

      listen_type = g_type_register_static (G_TYPE_OBJECT, 
                                            "BlListen",
                                            &listen_info, 0);
    }
  
  return listen_type;
}

static void
bl_listen_class_init (BlListenClass *class)
{
  GObjectClass *object_class;

  parent_class = g_type_class_peek_parent (class);

  object_class = G_OBJECT_CLASS (class);

  object_class->finalize = bl_listen_finalize;
}

static void
bl_listen_init (BlListen *listen)
{
  listen->sock = -1;
  listen->ccc  = NULL;
}

static void
bl_listen_finalize (GObject *object)
{
  BlListen *listen;

  listen = BL_LISTEN (object);

  if (listen->sock > -1)
    {
      close (listen->sock);
      listen->sock = -1;
    }

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

BlListen * 
bl_listen_new (gint   port,
               BlCcc *ccc)
{
  BlListen           *listener;
  struct sockaddr_in  addr;
  gint                sock;
  gint                i;

  g_return_val_if_fail (port > 0, NULL);

  if ((sock = socket (PF_INET, SOCK_STREAM, IPPROTO_IP)) != -1)
    {
      i = 1;
      if ((setsockopt (sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof (i))) == -1)
	{
          close (sock);
          g_warning ("Failed to configure socket: %s", g_strerror (errno));
          return NULL;          
        }

      memset (&addr, 0, sizeof (addr));

      addr.sin_family      = PF_INET;
      addr.sin_addr.s_addr = htonl (INADDR_LOOPBACK);
      addr.sin_port        = htons (port);

      if ((bind (sock, (struct sockaddr *) &addr, sizeof (addr))) == -1)
        {
          close (sock);
          g_print ("Failed to bind to socket: %s\n", g_strerror (errno));
          return NULL;
        }
    }
  else
    {
      g_warning ("Failed to open socket: %s", g_strerror (errno));
      return NULL;
    }

  if (listen (sock, 1) != 0)
    {
      g_warning ("Failed to put socket into the listening state: %s", 
                 g_strerror (errno));
      close (sock);
      return NULL;
    }

  listener = BL_LISTEN (g_object_new (BL_TYPE_LISTEN, NULL));

  listener->sock = sock;
  listener->ccc  = ccc;
  g_object_add_weak_pointer (G_OBJECT (ccc), (gpointer *) &listener->ccc);

  g_thread_create ((GThreadFunc) bl_accept_new, listener, FALSE, NULL);

  return listener;
}

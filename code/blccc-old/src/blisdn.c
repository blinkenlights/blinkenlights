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

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <gtk/gtk.h>

#include "blccc.h"
#include "bltypes.h"
#include "blisdn.h"

#define POLL_TIMEOUT 50


static void      bl_isdn_class_init  (BlIsdnClass *class);
static void      bl_isdn_init        (BlIsdn      *isdn);
static void      bl_isdn_destroy     (GtkObject    *object);

static gboolean  udp_prepare  (gpointer  source_data,
                               GTimeVal *current_time,
                               gint     *timeout,
                               gpointer  user_data);
static gboolean  udp_check    (gpointer  source_data,
                               GTimeVal *current_time,
                               gpointer  user_data);
static gboolean  udp_dispatch (gpointer  source_data, 
                               GTimeVal *dispatch_time,
                               gpointer  user_data);


static GSourceFuncs udp_funcs =
{
  udp_prepare,
  udp_check,
  udp_dispatch,
  NULL
};

enum 
{
  STATE_CHANGED,
  DIAL_TONE,
  LAST_SIGNAL
};
static guint bl_isdn_signals[LAST_SIGNAL] = { 0 };

static GtkObjectClass *parent_class = NULL;


GtkType
bl_isdn_get_type (void)
{
  static GtkType isdn_type = 0;

  if (!isdn_type)
    {
      GtkTypeInfo isdn_info =
      {
	"BlIsdn",
	sizeof (BlIsdn),
	sizeof (BlIsdnClass),
	(GtkClassInitFunc) bl_isdn_class_init,
	(GtkObjectInitFunc) bl_isdn_init,
	/* reserved_1 */ NULL,
	/* reserved_2 */ NULL,
        (GtkClassInitFunc) NULL
      };

      isdn_type = gtk_type_unique (gtk_object_get_type (), &isdn_info);
    }
  
  return isdn_type;
}

static void
bl_isdn_class_init (BlIsdnClass *class)
{
  GtkObjectClass *object_class;

  parent_class = gtk_type_class (gtk_object_get_type ());

  object_class = GTK_OBJECT_CLASS (class);

  bl_isdn_signals[STATE_CHANGED] = 
    gtk_signal_new ("state_changed",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (BlIsdnClass, state_changed),
		    gtk_signal_default_marshaller,
                    GTK_TYPE_NONE, 0);
  bl_isdn_signals[DIAL_TONE] = 
    gtk_signal_new ("dial_tone",
		    GTK_RUN_FIRST,
		    object_class->type,
		    GTK_SIGNAL_OFFSET (BlIsdnClass, dial_tone),
		    gtk_marshal_NONE__INT_INT,
                    GTK_TYPE_NONE, 2, GTK_TYPE_INT, GTK_TYPE_INT);

  gtk_object_class_add_signals (object_class, bl_isdn_signals, LAST_SIGNAL);
  
  class->state_changed = NULL;
  class->dial_tone     = NULL;

  object_class->destroy = bl_isdn_destroy;
}

static void
bl_isdn_init (BlIsdn *isdn)
{
  isdn->sock              = -1;
  isdn->sock_initialized  = FALSE;
  isdn->state_initialized = FALSE;
  isdn->packet_pending    = FALSE;
  isdn->no_packets        = 0;

  isdn->line0_offhook     = FALSE;
  isdn->line1_offhook     = FALSE;
}

static void
bl_isdn_destroy (GtkObject *object)
{
  BlIsdn *isdn;

  isdn = BL_ISDN (object);

  if (isdn->sock > -1)
    {
      close (isdn->sock);
      isdn->sock = -1;
    }

  if (GTK_OBJECT_CLASS (parent_class)->destroy)
    GTK_OBJECT_CLASS (parent_class)->destroy (object);
}

static gboolean
udp_prepare (gpointer  source_data,
             GTimeVal *current_time,
             gint     *timeout,
             gpointer  user_data)
{
  BlIsdn         *isdn = BL_ISDN (source_data);
  fd_set          set;
  struct timeval  tv;

  FD_ZERO (&set);
  FD_SET (isdn->sock, &set);
  tv.tv_sec  = 0;
  tv.tv_usec = 0;

  isdn->packet_pending = (select (isdn->sock + 1, &set, NULL, NULL, &tv) > 0);

  if (timeout)
    *timeout = POLL_TIMEOUT;

  if (isdn->packet_pending)
    isdn->no_packets = 0;
  else
    isdn->no_packets++;

  if (isdn->no_packets == (5000 / POLL_TIMEOUT))
    {
      isdn->line0_offhook = FALSE;
      isdn->line1_offhook = FALSE;

      gtk_signal_emit (GTK_OBJECT (isdn), 
                       bl_isdn_signals[STATE_CHANGED], NULL);
    }

  return isdn->packet_pending;
}

static gboolean
udp_check (gpointer  source_data,
           GTimeVal *current_time,
           gpointer  user_data)
{
  BlIsdn *isdn = BL_ISDN (source_data);

  return isdn->packet_pending;
}

static gboolean
udp_dispatch (gpointer  source_data,
              GTimeVal *dispatch_time,
              gpointer  user_data)
{
  BlIsdn         *isdn = BL_ISDN (source_data);
  guchar          packet[2];
  gboolean        line0_offhook;
  gboolean        line1_offhook;
  fd_set          set;
  struct timeval  tv;

  isdn->packet_pending = FALSE;

  FD_ZERO (&set);
  FD_SET (isdn->sock, &set);
  tv.tv_sec  = 0;
  tv.tv_usec = 0;

  /* on first call empty the queue */
  if (!isdn->sock_initialized)
    {
      while (select (isdn->sock + 1, &set, NULL, NULL, &tv) > 0)
        {
          FD_ZERO (&set);
          FD_SET (isdn->sock, &set);
          tv.tv_sec  = 0;
          tv.tv_usec = 0;
          
          recv (isdn->sock, packet, 2, 0);
        }
      isdn->sock_initialized = TRUE;
    }

  while (select (isdn->sock + 1, &set, NULL, NULL, &tv) > 0)
    {
      FD_ZERO (&set);
      FD_SET (isdn->sock, &set);
      tv.tv_sec  = 0;
      tv.tv_usec = 0;

      if (recv (isdn->sock, packet, 2, 0) != 2)
        continue;

      line0_offhook = (packet[0] != 0x0);
      line1_offhook = (packet[1] != 0x0);
      
      if (!isdn->state_initialized ||
          isdn->line0_offhook != line0_offhook ||
          isdn->line1_offhook != line1_offhook)
        {
          isdn->state_initialized = TRUE;
          isdn->line0_offhook = line0_offhook;
          isdn->line1_offhook = line1_offhook;

          gtk_signal_emit (GTK_OBJECT (isdn), 
                           bl_isdn_signals[STATE_CHANGED], NULL);
        }

      if ((isdn->line0_offhook && packet[0] != 0xFF) ||
          (isdn->line1_offhook && packet[1] != 0xFF))
        {
          gtk_signal_emit (GTK_OBJECT (isdn), 
                           bl_isdn_signals[DIAL_TONE], 
                           (gint) (isdn->line0_offhook ? packet[0] : 0), 
                           (gint) (isdn->line1_offhook ? packet[1] : 0),
                           NULL);
        }
    }
  
  return TRUE;
}

BlIsdn * 
bl_isdn_new (gint udp_port)
{
  BlIsdn             *isdn;
  struct sockaddr_in  addr;
  gint                sock;
  gint                i;

  g_return_val_if_fail (udp_port > 0, NULL);

  if ((sock = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP)) != -1)
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
      addr.sin_addr.s_addr = INADDR_ANY;
      addr.sin_port        = htons (udp_port);

      if ((bind (sock, (struct sockaddr *) &addr, sizeof (addr))) == -1)
        {
          close (sock);
          g_warning ("Failed to bind to socket: %s", g_strerror (errno));
          return NULL;
        }
    }
  else
    {
      g_warning ("Failed to open socket: %s", g_strerror (errno));
      return NULL;
    }

  isdn = BL_ISDN (gtk_object_new (BL_TYPE_ISDN, NULL));

  isdn->sock = sock;

  g_source_add (G_PRIORITY_DEFAULT, FALSE, &udp_funcs, isdn, NULL, NULL);

  return isdn;
}

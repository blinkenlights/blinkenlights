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
#include <fcntl.h>
#include <sys/types.h>

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <glib-object.h>

#ifdef G_OS_WIN32
#include <winsock2.h>
#ifndef socklen_t
#define socklen_t unsigned int
#endif
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif

#include "btypes.h"
#include "bpacket.h"
#include "breceiver.h"
#include "bsocket.h"


enum
{
  PROP_0,
  PROP_CALLBACK,
  PROP_CALLBACK_DATA
};


static void     b_receiver_class_init   (BReceiverClass *class);
static void     b_receiver_init         (BReceiver      *receiver);
static void     b_receiver_finalize     (GObject        *object);
static void     b_receiver_set_property (GObject        *object,
                                         guint           property_id,
                                         const GValue   *value,
                                         GParamSpec     *pspec);
static gboolean b_receiver_io_func      (GIOChannel     *io,
                                         GIOCondition    cond,
                                         gpointer        data);


static GObjectClass *parent_class = NULL;


GType
b_receiver_get_type (void)
{
  static GType receiver_type = 0;

  if (!receiver_type)
    {
      static const GTypeInfo receiver_info =
      {
        sizeof (BReceiverClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_receiver_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BReceiver),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_receiver_init,
      };

      receiver_type = g_type_register_static (G_TYPE_OBJECT,
                                              "BReceiver", &receiver_info, 0);
    }

  return receiver_type;
}

static void
b_receiver_class_init (BReceiverClass *class)
{
  GObjectClass *object_class;

  parent_class = g_type_class_peek_parent (class);
  object_class = G_OBJECT_CLASS (class);

  object_class->finalize     = b_receiver_finalize;
  object_class->set_property = b_receiver_set_property;

  g_object_class_install_property (object_class, PROP_CALLBACK,
                                   g_param_spec_pointer ("callback",
                                                         NULL, NULL,
                                                         G_PARAM_CONSTRUCT_ONLY |
                                                         G_PARAM_WRITABLE));
  g_object_class_install_property (object_class, PROP_CALLBACK_DATA,
                                   g_param_spec_pointer ("callback_data",
                                                         NULL, NULL,
                                                         G_PARAM_CONSTRUCT_ONLY |
                                                         G_PARAM_WRITABLE));
}

static void
b_receiver_init (BReceiver *receiver)
{
  receiver->io_channel    = NULL;
  receiver->watch_source  = 0;
  receiver->callback      = NULL;
  receiver->callback_data = NULL;
}

static void
b_receiver_finalize (GObject *object)
{
  BReceiver *receiver = B_RECEIVER (object);

  if (receiver->io_channel)
    g_io_channel_unref (receiver->io_channel);

  receiver->callback = NULL;

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

static void
b_receiver_set_property (GObject      *object,
                         guint         property_id,
                         const GValue *value,
                         GParamSpec   *pspec)
{
  BReceiver *receiver = B_RECEIVER (object);

  switch (property_id)
    {
    case PROP_CALLBACK:
      receiver->callback = g_value_get_pointer (value);
      break;
    case PROP_CALLBACK_DATA:
      receiver->callback_data = g_value_get_pointer (value);
      break;
    default:
      G_OBJECT_WARN_INVALID_PROPERTY_ID (object, property_id, pspec);
      break;
    }
}


/**
 * b_receiver_new:
 * @callback: the function to call when a new frame arrives
 * @callback_data: data to pass to the @callback
 *
 * Creates a new #BReceiver object.
 *
 * Return value: a newly allocate #BReceiver object
 **/
BReceiver *
b_receiver_new (BReceiverCallback callback,
                gpointer          callback_data)
{
  return B_RECEIVER (g_object_new (B_TYPE_RECEIVER,
                                   "callback",      callback,
                                   "callback_data", callback_data,
                                   NULL));
}

/**
 * b_receiver_listen:
 * @receiver: a #BReceiver object
 * @port: the UDP port to listen to
 *
 * Causes the @receiver to start to listen to the specified UDP
 * port. For each successfully received Blinkenlights packet, the
 * packet will be converted to host byteorder and the callback that
 * was specified on b_receiver_new() will be called.
 *
 * Return value: %TRUE if the receiver listens to @port, %FALSE otherwise
 **/
gboolean
b_receiver_listen (BReceiver *receiver,
                   gint       port)
{
  struct sockaddr_in  local_address;
  gint                status;
  gint                listen_fd;
  GError             *error = NULL;

  g_return_val_if_fail (B_IS_RECEIVER (receiver), FALSE);
  g_return_val_if_fail (receiver->io_channel == NULL, FALSE);

  listen_fd = b_socket_udp_new (B_SO_REUSEADDR, &error);
  if (listen_fd == -1)
    {
      g_printerr ("%s\n", error->message);
      g_error_free (error);
      return FALSE;
    }

  local_address.sin_family      = PF_INET;
  local_address.sin_port        = g_htons (port);
  local_address.sin_addr.s_addr = INADDR_ANY;

  status = bind (listen_fd,
                 (struct sockaddr *) &local_address, sizeof (local_address));
  if (status == -1)
    {
      g_printerr ("BReceiver: Can't bind local address");
      close (listen_fd);
      return FALSE;
    }

#ifndef G_OS_WIN32
  status = fcntl (listen_fd, F_SETFL, O_NONBLOCK);
  if (status == -1)
    {
      g_printerr ("BReceiver: Can't set non-blocking mode on socket");
      close (listen_fd);
      return FALSE;
    }
#endif

  b_receiver_listen_fd (receiver, listen_fd);

  g_io_channel_set_close_on_unref (receiver->io_channel, TRUE);

  return TRUE;
}

/**
 * b_receiver_listen_fd:
 * @receiver: a #BReceiver object
 * @fd: a file descriptor
 *
 * Causes the @receiver to start to listen to the given file descriptor.
 * This function is rarely useful, it is used internally by #BProxyClient.
 *
 * Return value: %TRUE on success
 **/
gboolean
b_receiver_listen_fd (BReceiver *receiver,
                      gint       fd)
{
  g_return_val_if_fail (B_IS_RECEIVER (receiver), FALSE);
  g_return_val_if_fail (receiver->io_channel == NULL, FALSE);

  receiver->io_channel = g_io_channel_unix_new (fd);

  g_io_channel_set_encoding (receiver->io_channel, NULL, NULL);

  receiver->watch_source = g_io_add_watch (receiver->io_channel,
                                           G_IO_IN | G_IO_PRI,
                                           b_receiver_io_func, receiver);

  return TRUE;
}

/**
 * b_receiver_stop:
 * @receiver: a #BReceiver object
 *
 * Makes the @receiver stop listening.
 **/
void
b_receiver_stop (BReceiver *receiver)
{
  g_return_if_fail (B_IS_RECEIVER (receiver));

  if (!receiver->io_channel)
    return;

  if (receiver->watch_source)
    {
      g_source_remove (receiver->watch_source);
      receiver->watch_source = 0;
    }

  g_io_channel_unref (receiver->io_channel);
  receiver->io_channel = NULL;
}

static gboolean
b_receiver_io_func (GIOChannel   *io,
                    GIOCondition  cond,
                    gpointer      data)
{
  BReceiver          *receiver;
  mcu_frame_header_t *header;
  guchar              buf[0xfff];
  BPacket            *packet = NULL;
  BPacket            *fake   = NULL;
  BPacket            *new    = NULL;
  gssize              buf_read;
  gboolean            success = TRUE;

  gint                req_fd;
  struct sockaddr_in  req_addr;
  socklen_t           req_addr_length = sizeof (req_addr);

  receiver = B_RECEIVER (data);

  if (! receiver->callback)
    return TRUE;

  req_fd = g_io_channel_unix_get_fd (io);

  buf_read = recvfrom (req_fd, buf, sizeof (buf), 0,
                       (struct sockaddr *) &req_addr, &req_addr_length);

  if (buf_read < sizeof (BPacket))
    return TRUE;

  new = (BPacket *) buf;

  b_packet_ntoh (new);

  header = &new->header.mcu_frame_h;

  switch (header->magic)
    {
    case MAGIC_MCU_FRAME:
      if (buf_read < (sizeof (BPacket) +
                      header->width * header->height * header->channels))
        return TRUE;
      /*  else fallthru  */

    case MAGIC_HEARTBEAT:
      packet = new;
      break;

    case MAGIC_BLFRAME:
      {
	gint size;

	fake = b_packet_new (18, 8, 1, 1, &size);

	memcpy (fake->data, (guchar *) new + sizeof (BPacket), size);

	packet = fake;
      }
      break;

    default:
      g_printerr ("BReceiver: Unknown magic: %08x, dropping packet!",
		  new->header.mcu_frame_h.magic);
      return TRUE;
    }

  receiver->addr = req_addr.sin_addr.s_addr;
  receiver->port = req_addr.sin_port;

  success = receiver->callback (receiver, packet, receiver->callback_data);

  receiver->addr = 0;
  receiver->port = 0;

  if (fake)
    g_free (fake);

  return success;
}

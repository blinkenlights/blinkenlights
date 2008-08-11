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
#include "bobject.h"
#include "bpacket.h"
#include "bsender.h"
#include "bsocket.h"


typedef struct _BRecipient
{
  gint                 fd;
  struct sockaddr_in   addr;
  gchar               *hostname;
  gint                 port;
} BRecipient;


static void b_sender_class_init (BSenderClass *class);
static void b_sender_init 	(BSender      *sender);
static void b_sender_finalize 	(GObject      *object);

static BObjectClass *parent_class = NULL;


GType
b_sender_get_type (void)
{
  static GType sender_type = 0;

  if (!sender_type)
    {
      static const GTypeInfo sender_info =
      {
        sizeof (BSenderClass),
        NULL,           /* base_init */
        NULL,           /* base_finalize */
        (GClassInitFunc) b_sender_class_init,
        NULL,           /* class_finalize */
        NULL,           /* class_data */
        sizeof (BSender),
        0,              /* n_preallocs */
        (GInstanceInitFunc) b_sender_init,
      };

      sender_type = g_type_register_static (G_TYPE_OBJECT,
                                            "BSender", &sender_info, 0);
    }

  return sender_type;
}

static void
b_sender_class_init (BSenderClass *class)
{
  GObjectClass *object_class;

  parent_class = g_type_class_peek_parent (class);
  object_class = G_OBJECT_CLASS (class);

  object_class->finalize = b_sender_finalize;
}

static void
b_sender_init (BSender *sender)
{
  sender->recipients = NULL;
  sender->packet     = NULL;
  sender->size       = 0;
  sender->verbose    = FALSE;
}

static void
b_sender_finalize (GObject *object)
{
  BSender *sender;
  GList   *item;

  sender = B_SENDER (object);

  for (item = sender->recipients; item; item = item->next)
    {
      BRecipient *rec = item->data;

      if (!rec)
        continue;

      if (rec->fd > -1)
        close (rec->fd);

      g_free (rec);
    }

  g_list_free (sender->recipients);

  if (sender->packet)
    g_free (sender->packet);

  G_OBJECT_CLASS (parent_class)->finalize (object);
}

/**
 * b_sender_new:
 *
 * Creates a new #BSender object.
 *
 * Return value: a newly allocated #BSender object
 **/
BSender *
b_sender_new (void)
{
  return B_SENDER (g_object_new (B_TYPE_SENDER, NULL));
}

/**
 * b_sender_configure:
 * @sender: a #BSender object
 * @width: number of pixels per row
 * @height: number of pixels per column
 * @channels: number of channels per pixel
 * @maxval: the maximum value
 *
 * Prepares @sender to send Blinkenlights UDP packets with the given
 * parameters.
 *
 * Return value: %TRUE if @sender was successfully reconfigured,
 * %FALSE otherwise
 **/
gboolean
b_sender_configure (BSender *sender,
                    gint     width,
                    gint     height,
                    gint     channels,
                    gint     maxval)
{
  g_return_val_if_fail (B_IS_SENDER (sender), FALSE);
  g_return_val_if_fail (width > 0 && height > 0, FALSE);

  /* prepare packet */
  if (sender->packet)
    g_free (sender->packet);

  sender->packet = b_packet_new (width, height, channels, maxval,
                                 &sender->size);

  b_packet_hton (sender->packet);

  return TRUE;
}

/**
 * b_sender_add_recipient:
 * @sender: a #BSender object
 * @src_port:  originating UDP port (-1 to leave unspecified)
 * @dest_host: destination hostname or IP address
 * @dest_port: destination UDP port
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Tries to resolve @host and adds it to the @sender's list of recipients.
 *
 * Return value: %TRUE if the recipient was successfully added, %FALSE
 * otherwise
 **/
gboolean
b_sender_add_recipient (BSender      *sender,
                        gint          src_port,
                        const gchar  *dest_host,
                        gint          dest_port,
                        GError      **error)
{
  BRecipient         *rec;
  struct hostent     *dest;
  struct sockaddr_in  sock;

  g_return_val_if_fail (B_IS_SENDER (sender), FALSE);
  g_return_val_if_fail (dest_host != NULL && *dest_host != '\0', FALSE);
  g_return_val_if_fail (dest_port > 0, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  if (! b_socket_init (error))
    return FALSE;

  dest = gethostbyname (dest_host);
  if (dest == NULL)
    {
      g_set_error (error, 0, 0, "Unable to resolve host '%s'", dest_host);
      return FALSE;
    }

  rec = g_new0 (BRecipient, 1);
  rec->fd = -1;

  rec->addr.sin_family = AF_INET;
  rec->addr.sin_port   = g_htons (dest_port);

  memcpy (&rec->addr.sin_addr.s_addr, dest->h_addr_list[0], dest->h_length);

  rec->fd = b_socket_udp_new (B_SO_BROADCAST | B_SO_REUSEADDR, error);
  if (rec->fd < 0)
    return FALSE;

  if (src_port > 0)
    {
      sock.sin_addr.s_addr = INADDR_ANY;
      sock.sin_family      = AF_INET;
      sock.sin_port        = g_htons (src_port);

      if (bind (rec->fd, (struct sockaddr *) &sock, sizeof (sock)) < 0)
        {
          g_set_error (error, 0, 0,
                       "Couldn't bind socket for %s to local port %d: %s\n",
                       dest_host, src_port, g_strerror (errno));
          close (rec->fd);
          g_free (rec);
          return FALSE;
        }
    }

  if (connect (rec->fd,
               (struct sockaddr *) &rec->addr, sizeof (rec->addr)) < 0)
   {
      g_set_error (error, 0, 0,
                   "Couldn't connect socket for %s: %s\n",
                   dest_host, g_strerror (errno));
      close (rec->fd);
      g_free (rec);
      return FALSE;
   }

  rec->hostname = g_strdup (dest_host);
  rec->port     = dest_port;

  /* To avoid duplicate entries, first remove all recipients with
     same hostname and same port. */
  b_sender_remove_recipient (sender, rec->hostname, rec->port, NULL);

  sender->recipients = g_list_append (sender->recipients, rec);

  return TRUE;
}

/**
 * b_sender_remove_recipient:
 * @sender: a #BSender object
 * @dest_host: destination hostname or IP address
 * @dest_port: destination UDP port
 * @error: location to store the error occuring, or %NULL to ignore errors
 *
 * Removes all recipients with matching host and port from the
 * @sender's list of recipients.
 *
 * Return value: %TRUE if at least one matching recipient was found
 * and removed, %FALSE otherwise
 **/
gboolean
b_sender_remove_recipient (BSender      *sender,
                           const gchar  *dest_host,
                           gint          dest_port,
                           GError      **error)
{
  GList    *item;
  gboolean  done = FALSE;

  g_return_val_if_fail (B_IS_SENDER (sender), FALSE);
  g_return_val_if_fail (dest_host != NULL && *dest_host != '\0', FALSE);
  g_return_val_if_fail (dest_port > 0, FALSE);
  g_return_val_if_fail (error == NULL || *error == NULL, FALSE);

  for (item = sender->recipients; item; item = item->next)
    {
      BRecipient *rec = item->data;

      if (!rec)
        continue;

      if (rec->port == dest_port &&
          g_ascii_strcasecmp (rec->hostname, dest_host) == 0)
        {
          if (rec->fd > -1)
            close (rec->fd);

	  g_free (rec->hostname);
          g_free (rec);

          item->data = NULL;
          done = TRUE;
        }
    }

  if (done)
    sender->recipients = g_list_remove_all (sender->recipients, NULL);
  else
    g_set_error (error, 0, 0,
		 "No such host in recipient list: %s, port %d\n",
		 dest_host, dest_port);

  return done;
}

/**
 * b_sender_list_recipients:
 * @sender: a #BSender object
 *
 * Creates a list of strings that describe the @senders
 * recipients. You should free the strings as well as the list when
 * it is no longer needed.
 *
 * Return value: a newly allocated #GList of newly allocated strings
 **/
GList *
b_sender_list_recipients (BSender *sender)
{
  GList *item, *list = NULL;

  for (item = sender->recipients; item; item = item->next)
    {
      BRecipient *rec = item->data;

      if (!rec)
        continue;

      list = g_list_append (list,
                            g_strdup_printf ("%s:%d",
                                             rec->hostname, rec->port));
    }

  return list;
}

/**
 * b_sender_send_frame:
 * @sender: a #Bsender object
 * @data: the frame data to send
 *
 * Sends Blinkenlights packets to all recipients registered with
 * @sender. The @data should match the values of the last call to
 * b_sender_configure() for @sender.
 *
 * Return value: %FALSE in case of a fatal error, %TRUE otherwise
 **/
gboolean
b_sender_send_frame (BSender      *sender,
                     const guchar *data)
{
  GList  *item;
  gssize  packet_size = sender->size + sizeof (BPacket);

  g_return_val_if_fail (B_IS_SENDER (sender), FALSE);

  if (sender->packet == NULL)
    {
      g_warning ("Call b_sender_configure() before sending packages!");
      return FALSE;
    }

  if (data)
    memcpy (sender->packet->data, data, sender->size);
  else
    memset (sender->packet->data, 0, sender->size);

  /* ...send it. */
  for (item = sender->recipients; item; item = item->next)
    {
      BRecipient *rec = (BRecipient *) item->data;

      if (rec == NULL)
        continue;

      if (send (rec->fd, sender->packet, packet_size, 0) != packet_size)
        {
          if (sender->verbose)
            g_printerr ("Unable to send to %s: %s\n",
                        rec->hostname, g_strerror (errno));
        }
    }

  return TRUE;
}

/**
 * b_sender_send_heartbeat:
 * @sender: a #Bsender object
 *
 * This function is deprecated and might be removed in the future.
 * If you need the heartbeat functionality, use #BProxyClient instead.
 *
 * Return value: %TRUE always
 **/
gboolean
b_sender_send_heartbeat (BSender *sender)
{
  GList  *item;
  BPacket packet;

  g_return_val_if_fail (B_IS_SENDER (sender), FALSE);

  memset (&packet, 0, sizeof (BPacket));

  packet.header.heartbeat_h.magic = MAGIC_HEARTBEAT;

  b_packet_hton (&packet);

  for (item = sender->recipients; item; item = item->next)
    {
      BRecipient *rec = (BRecipient *) item->data;

      if (rec == NULL)
        continue;

      if (send (rec->fd, &packet, sizeof (BPacket), 0) != sizeof (BPacket))
        {
          if (sender->verbose)
            g_printerr ("Unable to send heartbeat to %s: %s\n",
                        rec->hostname, g_strerror (errno));
        }
    }

  return TRUE;
}

/**
 * b_sender_set_verbose:
 * @sender: a #Bsender object
 * @verbose: whether the @sender should do verbose error reporting or not
 *
 * In earlier BLib versions #BSender used to print a message to stderr
 * when send() failed. This message is now suppressed by default, but
 * you can get the old behaviour back using this function.
 **/
void
b_sender_set_verbose (BSender  *sender,
                      gboolean  verbose)
{
  g_return_if_fail (B_IS_SENDER (sender));

  sender->verbose = (verbose != FALSE);
}

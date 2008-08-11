/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
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


#ifndef __B_RECEIVER_H__
#define __B_RECEIVER_H__

G_BEGIN_DECLS

#define B_TYPE_RECEIVER            (b_receiver_get_type ())
#define B_RECEIVER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_RECEIVER, BReceiver))
#define B_RECEIVER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_RECEIVER, BReceiverClass))
#define B_IS_RECEIVER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_RECEIVER))
#define B_IS_RECEIVER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_RECEIVER))
#define B_RECEIVER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), B_TYPE_RECEIVER, BReceiverClass))


typedef struct _BReceiverClass BReceiverClass;

typedef gboolean (* BReceiverCallback) (BReceiver *receiver,
                                        BPacket   *packet,
                                        gpointer   callback_data);

struct _BReceiverClass
{
  GObjectClass       parent_class;
};

struct _BReceiver
{
  GObject            parent_instance;

  GIOChannel        *io_channel;
  guint              watch_source;

  BReceiverCallback  callback;
  gpointer           callback_data;

  gulong             addr;  /* in network byte order */
  gushort            port;  /* in network byte order */
};

GType        b_receiver_get_type  (void) G_GNUC_CONST;
BReceiver  * b_receiver_new   	  (BReceiverCallback  callback,
                                   gpointer           callback_data);
gboolean     b_receiver_listen    (BReceiver         *receiver,
                                   gint               port);
gboolean     b_receiver_listen_fd (BReceiver         *receiver,
                                   gint               fd);
void         b_receiver_stop      (BReceiver         *receiver);

G_END_DECLS

#endif /* __B_RECEIVER_H__ */

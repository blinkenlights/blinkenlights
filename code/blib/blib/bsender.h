/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (c) 2001-2002  The Blinkenlights Crew
 *                          Sven Neumann <sven@gimp.org>
 *                          Michael Natterer <mitch@gimp.org>
 *			   Daniel Mack <daniel@yoobay.net>
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

#ifndef __B_SENDER_H__
#define __B_SENDER_H__

#include <glib.h>

G_BEGIN_DECLS

#define B_TYPE_SENDER            (b_sender_get_type ())
#define B_SENDER(obj)            (G_TYPE_CHECK_INSTANCE_CAST ((obj), B_TYPE_SENDER, BSender))
#define B_SENDER_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST ((klass), B_TYPE_SENDER, BSenderClass))
#define B_IS_SENDER(obj)         (G_TYPE_CHECK_INSTANCE_TYPE ((obj), B_TYPE_SENDER))
#define B_IS_SENDER_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE ((klass), B_TYPE_SENDER))
#define B_SENDER_GET_CLASS(obj)  (G_TYPE_INSTANCE_GET_CLASS ((obj), B_TYPE_SENDER, BSenderClass))


typedef struct _BSenderClass BSenderClass;

struct _BSenderClass
{
  GObjectClass  parent_class;
};

struct _BSender
{
  GObject   parent_instance;

  GList    *recipients;

  BPacket  *packet;
  gsize     size;

  gboolean  verbose;
};

GType      b_sender_get_type         (void) G_GNUC_CONST;
BSender  * b_sender_new   	     (void);

gboolean   b_sender_add_recipient    (BSender       *sender,
                                      gint           src_port,
                                      const gchar   *dest_host,
                                      gint           dest_port,
                                      GError       **error);
gboolean   b_sender_remove_recipient (BSender       *sender,
                                      const gchar   *dest_host,
                                      gint           dest_port,
                                      GError       **error);
GList    * b_sender_list_recipients  (BSender *sender);

gboolean   b_sender_configure        (BSender       *sender,
                                      gint           width,
                                      gint           height,
                                      gint           channels,
                                      gint           maxval);
gboolean   b_sender_send_frame       (BSender       *sender,
                                      const guchar  *data);
void       b_sender_set_verbose      (BSender       *sender,
                                      gboolean       verbose);

#ifndef B_DISABLE_DEPRECATED
gboolean   b_sender_send_heartbeat   (BSender       *sender);
#endif

G_END_DECLS

#endif /* __B_SENDER_H__ */

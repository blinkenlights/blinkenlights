/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (C) 2002  The Blinkenlights Crew
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

#ifndef __B_PACKET_H__
#define __B_PACKET_H__

#include "blib/bprotocol.h"

G_BEGIN_DECLS

typedef union _BPacketHeader BPacketHeader;

union _BPacketHeader
{
  mcu_frame_header_t       mcu_frame_h;
  heartbeat_header_t       heartbeat_h;
  mcu_multiframe_header_t  mcu_multiframe_h;
};

struct _BPacket
{
  BPacketHeader      header;
  guchar             data[0];
};

BPacket * b_packet_new  (gint     width,
			 gint     height,
			 gint     channels,
			 gint     maxval,
			 guint    magic,
			 gsize   *data_size);
gsize     b_packet_size (BPacket *packet);
gsize     b_packet_multiframe_subframe_size(mcu_subframe_header_t *subframe_header);

void      b_packet_multiframe_hton (mcu_subframe_header_t *subframe_header);
void      b_packet_multiframe_ntoh (mcu_subframe_header_t *subframe_header);

void      b_packet_hton (BPacket *packet);
void      b_packet_ntoh (BPacket *packet);

G_END_DECLS

#endif  /* __B_PACKET_H__ */

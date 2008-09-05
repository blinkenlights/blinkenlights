/* blib - Library of useful things to hack the Blinkenlights
 *
 * Copyright (C) 2002  The Blinkenlights Crew
 *                     Sven Neumann <sven@gimp.org>
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

#include <glib-object.h>

#include "btypes.h"
#include "bpacket.h"


/**
 * b_packet_new:
 * @width: the number of pixels per row
 * @height: the number of pixels per column
 * @channels: the number of channels per pixels
 * @maxval: the maximum value
 * @data_size: returns the size of the packet data
 *
 * Allocates a new #BPacket structure and initializes it with the
 * given values. If @data_size is non-NULL the size of the data area
 * (in bytes) is returned via this variable.
 *
 * The packet should be freed with g_free() when it is not needed any
 * longer.
 *
 * Return value: a newly allocated #BPacket.
 **/
BPacket *
b_packet_new (gint  width,
              gint  height,
              gint  channels,
              gint  maxval,
              gint *data_size)
{
  BPacket *packet;
  gint     size;

  g_return_val_if_fail (width > 0, NULL);
  g_return_val_if_fail (height > 0, NULL);
  g_return_val_if_fail (channels > 0, NULL);
  g_return_val_if_fail (maxval > 0 && maxval <= 255, NULL);

  size = width * height * channels;

  packet = (BPacket *) g_new0 (guchar, sizeof (BPacket) + size);

  packet->header.mcu_frame_h.magic    = MAGIC_MCU_FRAME;
  packet->header.mcu_frame_h.width    = width;
  packet->header.mcu_frame_h.height   = height;
  packet->header.mcu_frame_h.channels = channels;
  packet->header.mcu_frame_h.maxval   = maxval;

  if (data_size)
    *data_size = size;

  return packet;
}

gsize
b_packet_size (BPacket *packet)
{
  guint32 *magic;

  g_return_val_if_fail (packet != NULL, 0);

  magic = (guint32 *) packet;

  switch (*magic)
    {
    case MAGIC_MCU_FRAME:
      {
        mcu_frame_header_t *header = &packet->header.mcu_frame_h;

        return (sizeof (BPacket) +
                header->width * header->height * header->channels);
      }

    default:
      return (sizeof (BPacket));
    }
}


/*  byte order conversions  */

/**
 * b_packet_hton:
 * @packet: pointer to a #BPacket
 *
 * Converts all members of @packet from host to network byteorder.
 **/
void
b_packet_hton (BPacket *packet)
{
  guint32 *magic = (guint32 *) packet;

  switch (*magic)
    {
    case MAGIC_MCU_FRAME:
      {
        mcu_frame_header_t *header = &packet->header.mcu_frame_h;

        header->magic    = g_htonl (header->magic);
        header->width    = g_htons (header->width);
        header->height   = g_htons (header->height);
        header->channels = g_htons (header->channels);
        header->maxval   = g_htons (header->maxval);
      }
      break;

    case MAGIC_HEARTBEAT:
      {
        heartbeat_header_t *header = &packet->header.heartbeat_h;

        header->magic    = g_htonl (header->magic);
        header->version  = g_htons (header->version);
      }
      break;

    default:
      *magic = g_htonl (magic);
      break;
    }
}

/**
 * b_packet_ntoh:
 * @packet: pointer to a #BPacket
 *
 * Converts all members of @packet from network to host byteorder.
 **/
void
b_packet_ntoh (BPacket *packet)
{
  guint32 *magic = (guint32 *) packet;

  *magic = g_ntohl (*magic);

  switch (*magic)
    {
    case MAGIC_MCU_FRAME:
      {
        mcu_frame_header_t *header = &packet->header.mcu_frame_h;

        header->width    = g_ntohs (header->width);
        header->height   = g_ntohs (header->height);
        header->channels = g_ntohs (header->channels);
        header->maxval   = g_ntohs (header->maxval);
      }
      break;

    case MAGIC_HEARTBEAT:
      {
        heartbeat_header_t *header = &packet->header.heartbeat_h;

        header->version  = g_ntohs (header->version);
      }
      break;

    default:
      break;
    }
}

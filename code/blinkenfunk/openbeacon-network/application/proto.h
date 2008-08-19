/***************************************************************
 *
 * OpenBeacon.org - OpenBeacon link layer protocol
 *
 * Copyright 2007 Milosch Meriac <meriac@openbeacon.de>
 *
 ***************************************************************

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; version 2.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

#ifndef __PROTO_H__
#define __PROTO_H__

#include "openbeacon.h"

#define FIFO_DEPTH 256

enum {
  RF_CMD_SET_LAMP_ID
};

typedef struct
{
  char cmd;
  char param;
  char payload[28];
  unsigned short crc;
} __attribute__ ((packed)) BRFPacket;

extern void vInitProtocolLayer (void);
extern void vnRFTransmitPacket(BRFPacket *pkg);
extern int PtSetFifoLifetimeSeconds (int Seconds);
extern int PtGetFifoLifetimeSeconds (void);
extern void PtDumpUIntToUSB (unsigned int data);
extern void PtDumpStringToUSB (const char *text);

#endif/*__PROTO_H__*/

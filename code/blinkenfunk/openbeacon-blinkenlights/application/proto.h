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
#include "usbshell.h"

#define PTINITNRFFRONTEND_RESETFIFO 0x01
#define PTINITNRFFRONTEND_INIT 0x02

#define GAMMA_DEFAULT	200
#define FIFO_DEPTH	256
#define RF_PAYLOAD_SIZE	22
enum
{
  RF_CMD_SET_VALUES,
  RF_CMD_SET_LAMP_ID,
  RF_CMD_SET_GAMMA,
  RF_CMD_WRITE_CONFIG,
  RF_CMD_SET_JITTER,
  RF_CMD_SEND_STATISTICS,
  RF_CMD_SET_DIMMER_DELAY,
  RF_CMD_SET_DIMMER_CONTROL,
  RF_CMD_PING,
  RF_CMD_ENTER_UPDATE_MODE = 0x3f
};

typedef struct
{
  unsigned char cmd;
  unsigned short mac;
  unsigned char wmcu_id;

  union
  {
    unsigned char payload[RF_PAYLOAD_SIZE];

    struct
    {
      unsigned char id;
      unsigned char wmcu_id;
    } PACKED set_lamp_id;

    struct
    {
      unsigned char block;
      unsigned short val[8];
    } PACKED set_gamma;

    struct
    {
      unsigned short jitter;
    } PACKED set_jitter;

    struct
    {
      unsigned short emi_pulses;
      unsigned long packet_count;
      unsigned long pings_lost;
      unsigned long fw_version;
    } PACKED statistics;

    struct
    {
      unsigned short delay;
    } PACKED set_delay;

    struct
    {
      unsigned char off;
    } PACKED dimmer_control;

    struct
    {
      unsigned int sequence;
    } PACKED ping;

  } PACKED;			/* union */

  unsigned int sequence;
  unsigned short crc;
} PACKED BRFPacket;

extern void vInitProtocolLayer (void);
extern int PtSetFifoLifetimeSeconds (int Seconds);
extern int PtGetFifoLifetimeSeconds (void);
extern void PtInitNrfFrontend (int ResetType);
extern unsigned int packet_count;
extern unsigned int last_sequence;
extern unsigned int last_ping_seq;
extern unsigned int pings_lost;

#endif/*__PROTO_H__*/

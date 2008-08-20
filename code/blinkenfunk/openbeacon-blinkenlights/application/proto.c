/***************************************************************
 *
 * OpenBeacon.org - OpenBeacon dimmer link layer protocol
 *
 * Copyright 2008 Milosch Meriac <meriac@openbeacon.de>
 *                Daniel Mack <daniel@caiaq.de>
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
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <string.h>
#include <math.h>
#include <board.h>
#include <beacontypes.h>
#include <USB-CDC.h>
#include "led.h"
#include "xxtea.h"
#include "proto.h"
#include "nRF24L01/nRF_HW.h"
#include "nRF24L01/nRF_CMD.h"
#include "nRF24L01/nRF_API.h"
#include "dimmer.h"
#include "env.h"
#include "debug_print.h"

static BRFPacket pkt;
const unsigned char broadcast_mac[NRF_MAX_MAC_SIZE] = { 'D', 'E', 'C', 'A', 'D' };

#define BLINK_INTERVAL_MS (50 / portTICK_RATE_MS)

static inline s_int8_t
PtInitNRF (void)
{
  if (!nRFAPI_Init
      (DEFAULT_CHANNEL, broadcast_mac, sizeof (broadcast_mac),
       ENABLED_NRF_FEATURES))
    return 0;

  nRFAPI_SetPipeSizeRX (0, sizeof(pkt));
  nRFAPI_SetTxPower (3);
  nRFAPI_SetRxMode (1);
  nRFCMD_CE (1);

  return 1;
}

static inline unsigned short
swapshort (unsigned short src)
{
  return (src >> 8) | (src << 8);
}

static inline unsigned long
swaplong (unsigned long src)
{
  return (src >> 24) | 
  	 (src << 24) | 
	 ((src >> 8) & 0x0000FF00) |
	 ((src << 8) & 0x00FF0000);
}

static void
shuffle_tx_byteorder (unsigned long *v, int len)
{
  while(len--) {
    *v = swaplong(*v);
    v++;
  }
}

static inline short
crc16 (const unsigned char *buffer, int size)
{
  unsigned short crc = 0xFFFF;

  if (buffer && size)
    while (size--)
      {
	crc = (crc >> 8) | (crc << 8);
	crc ^= *buffer++;
	crc ^= ((unsigned char) crc) >> 4;
	crc ^= crc << 12;
	crc ^= (crc & 0xFF) << 5;
      }

  return crc;
}

static inline void bParsePacket(void)
{
  DumpStringToUSB ("RX cmd: ");
  DumpUIntToUSB (pkt.cmd);
  DumpStringToUSB ("\n\r");
  
  switch (pkt.cmd)
    {
      case RF_CMD_SET_VALUES:
      {
        char v;
	
	if (env.e.lamp_id * 2 >= RF_PAYLOAD_SIZE)
	  break;
	
	v = pkt.payload[env.e.lamp_id / 2];
	if (env.e.lamp_id & 1)
	  v >>= 4;

	v &= 0xf;
        
	DumpStringToUSB ("new lamp val: ");
	DumpUIntToUSB (v);
        DumpStringToUSB ("\n\r");

	vUpdateDimmer((v * 500) + 2000);
        break;
      }
      case RF_CMD_SET_LAMP_ID:
        DumpStringToUSB ("new lamp id: ");
	DumpUIntToUSB (pkt.set_lamp_id.id);
        DumpStringToUSB (", new lamp line: ");
	DumpUIntToUSB (pkt.set_lamp_id.line);
        DumpStringToUSB ("\n\r");

	env.e.lamp_id = pkt.set_lamp_id.id;
	env.e.line_id = pkt.set_lamp_id.line;
	// env_store();

        break;
      case RF_CMD_SET_GAMMA:
        break;
      case RF_CMD_WRITE_GAMMA:
        break;
      case RF_CMD_SET_JITTER:
        break;
    }
}

void
vnRFtaskRx (void *parameter)
{
  u_int16_t crc;
  (void) parameter;
  int DidBlink = 0;
  portTickType Ticks = 0;

  if (!PtInitNRF ())
    return;

  for (;;)
    {
      if (nRFCMD_WaitRx (10))
	{
	  do
	    {
	      // read packet from nRF chip
	      nRFCMD_RegReadBuf (RD_RX_PLOAD, (unsigned char *) &pkt, sizeof(pkt));

	      // adjust byte order and decode
	      shuffle_tx_byteorder ((unsigned long *) &pkt, sizeof (pkt) / sizeof(long));
	      xxtea_decode ((long *) &pkt, sizeof(pkt) / sizeof(long));
	      shuffle_tx_byteorder ((unsigned long *) &pkt, sizeof (pkt) / sizeof(long));

	      // verify the crc checksum
	      crc = crc16 ((unsigned char *) &pkt, sizeof(pkt) - sizeof(pkt.crc));

	      if (crc != swapshort(pkt.crc))
	        continue;

              // valid paket
              if (!DidBlink)
                {
		  vLedSetGreen (1);
		  Ticks = xTaskGetTickCount ();
		  DidBlink = 1;
		}

		bParsePacket();
	    }
	  while ((nRFAPI_GetFifoStatus () & FIFO_RX_EMPTY) == 0);
	} // if

      nRFAPI_ClearIRQ (MASK_IRQ_FLAGS);

      if (DidBlink && ((xTaskGetTickCount () - Ticks) > BLINK_INTERVAL_MS))
	{
	  DidBlink = 0;
	  vLedSetGreen (0);
	}
    } // for (;;)
}

void
vnRFtaskCmd (void *parameter)
{
  (void) parameter;
  portCHAR c;
  int Percent = 2000;
  bool_t Changed;

  /* Init Dimmer and wait till initial frequency is measured */
  vInitDimmer ();
  while (!dimmer_line_hz_enabled())
    vTaskDelay (250 / portTICK_RATE_MS);
  vUpdateDimmer (Percent);


  while (1)
    {
      if (vUSBRecvByte (&c, sizeof (c), 100))
	{
	  Changed = false;

	  if (c >= 'a' && c <= 'q')
	    {
	      Percent = 2000 + (c - 'a') * 500;
	      Changed = pdTRUE;
	    }
	  else if (c >= '0' && c <= '9')
	    {
	      Percent = 2000 + (c - '0') * 889;
	      Changed = pdTRUE;
	    }
	  else
	    switch (c)
	      {
	      case '/':
		if (Percent % 100)
		  Percent -= Percent % 100;
		else
		  Percent -= 100;
		Changed = pdTRUE;
		break;
	      case '*':
		if (Percent % 100)
		  Percent += 100 - (Percent % 100);
		else
		  Percent += 100;
		Changed = pdTRUE;
		break;
	      case '+':
		Percent++;
		Changed = pdTRUE;
		break;
	      case '-':
		Percent--;
		Changed = pdTRUE;
		break;
	      }

	  if (Changed)
	    {
	      DumpStringToUSB ("DIM=");
	      if (Percent > 10000)
		Percent = 10000;
	      else if (Percent < 2000)
		Percent = 2000;
	      DumpUIntToUSB (Percent);
	      DumpStringToUSB ("%%\n\r");
	      vUpdateDimmer (Percent);
	    }
	}
    }
}

void
vInitProtocolLayer (void)
{
  xTaskCreate (vnRFtaskRx, (signed portCHAR *) "nRF_Rx", TASK_NRF_STACK,
	       NULL, TASK_NRF_PRIORITY, NULL);

  xTaskCreate (vnRFtaskCmd, (signed portCHAR *) "nRF_Cmd", TASK_CMD_STACK,
	       NULL, TASK_CMD_PRIORITY, NULL);
}

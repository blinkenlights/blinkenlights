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
#include <FreeRTOS.h>
#include <semphr.h>
#include <task.h>
#include <string.h>
#include <board.h>
#include <beacontypes.h>
#include <crc32.h>
#include "led.h"
#include "xxtea.h"
#include "proto.h"
#include "bprotocol.h"
#include "nRF24L01/nRF_HW.h"
#include "nRF24L01/nRF_CMD.h"
#include "nRF24L01/nRF_API.h"
#include "debug_printf.h"
#include "env.h"

static BRFPacket rxpkg;
unsigned int rf_sent_broadcast, rf_sent_unicast, rf_rec;
const unsigned char broadcast_mac[NRF_MAX_MAC_SIZE] =
  { 'D', 'E', 'C', 'A', 'D' };

static inline s_int8_t
PtInitNRF (void)
{
  if (!nRFAPI_Init (DEFAULT_CHANNEL, broadcast_mac,
		    sizeof (broadcast_mac), ENABLED_NRF_FEATURES))
    return 0;

  nRFAPI_SetPipeSizeRX (0, sizeof (rxpkg));
  nRFAPI_SetTxPower (3);
  nRFAPI_SetRxMode (0);
  nRFCMD_CE (0);

  return 1;
}

void
shuffle_tx_byteorder (unsigned long *v, int len)
{
  while (len--)
    {
      *v = swaplong (*v);
      v++;
    }
}

void
vnRFTransmitPacket (BRFPacket * pkg)
{
  vTaskDelay (env.e.rf_delay / portTICK_RATE_MS);

  /* turn on redLED for TX indication */
  vLedSetRed (1);

  /* disable receive mode */
  nRFCMD_CE (0);

  /* set TX mode */
  nRFAPI_SetRxMode (0);

  /* update the sequence */
  if (sequence_seed == 0)
    return;

  if (pkg->mac == 0xffff)
    rf_sent_broadcast++;
  else
    rf_sent_unicast++;

  pkg->sequence = sequence_seed + (xTaskGetTickCount () / portTICK_RATE_MS);

  /* update crc */
  pkg->crc =
    swaplong (crc32
	      ((unsigned char *) pkg, sizeof (*pkg) - sizeof (pkg->crc)));

  /* encrypt the data */
  shuffle_tx_byteorder ((unsigned long *) pkg, sizeof (*pkg) / sizeof (long));
  xxtea_encode ((long *) pkg, sizeof (*pkg) / sizeof (long));
  shuffle_tx_byteorder ((unsigned long *) pkg, sizeof (*pkg) / sizeof (long));

  /* upload data to nRF24L01 */
  //hex_dump((unsigned char *) pkg, 0, sizeof(*pkg));
  nRFAPI_TX ((unsigned char *) pkg, sizeof (*pkg));

  /* transmit data */
  nRFCMD_CE (1);

  /* wait till packet is transmitted */
  vTaskDelay (10 / portTICK_RATE_MS);

  /* switch to RX mode again */
  nRFAPI_SetRxMode (1);

  /* turn off red TX indication LED */
  vLedSetRed (0);
}

void
vnRFtaskRx (void *parameter)
{
  u_int32_t crc;
  u_int8_t status;

  if (!PtInitNRF ())
    return;

  /* FIXME!!! THIS IS RACY AND NEEDS A LOCK!!! */

  for (;;)
    {
      status = nRFAPI_GetFifoStatus ();
      /* living in a paranoid world ;-) */
      if (status & FIFO_TX_FULL)
	nRFAPI_FlushTX ();

      if ((status & FIFO_RX_FULL) || nRFCMD_WaitRx (10))
	{
	  vLedSetRed (1);

	  do
	    {
	      /* read packet from nRF chip */
	      nRFCMD_RegReadBuf (RD_RX_PLOAD, (unsigned char *) &rxpkg,
				 sizeof (rxpkg));

	      /* adjust byte order and decode */
	      shuffle_tx_byteorder ((unsigned long *) &rxpkg,
				    sizeof (rxpkg) / sizeof (long));
	      xxtea_decode ((long *) &rxpkg, sizeof (rxpkg) / sizeof (long));
	      shuffle_tx_byteorder ((unsigned long *) &rxpkg,
				    sizeof (rxpkg) / sizeof (long));

	      /* verify the crc checksum */
	      crc =
		crc32 ((unsigned char *) &rxpkg,
		       sizeof (rxpkg) - sizeof (rxpkg.crc));
	      if ((crc == swaplong (rxpkg.crc)) &&
		  (rxpkg.wmcu_id == env.e.mcu_id) &&
		  (rxpkg.cmd & RF_PKG_SENT_BY_DIMMER))
		{
		  //debug_printf("dumping received packet:\n");
		  //hex_dump((unsigned char *) &rxpkg, 0, sizeof(rxpkg));
		  b_parse_rfrx_pkg (&rxpkg);
		  rf_rec++;
		}
	    }
	  while ((nRFAPI_GetFifoStatus () & FIFO_RX_EMPTY) == 0);

	  vLedSetRed (0);
	}

      /* did I already mention the paranoid world thing? */
      nRFAPI_ClearIRQ (MASK_IRQ_FLAGS);
    }
}

void
vInitProtocolLayer (void)
{
  rf_rec = rf_sent_unicast = rf_sent_broadcast = 0;
  xTaskCreate (vnRFtaskRx, (signed portCHAR *) "nRF_Rx", TASK_NRF_STACK,
	       NULL, TASK_NRF_PRIORITY, NULL);
}

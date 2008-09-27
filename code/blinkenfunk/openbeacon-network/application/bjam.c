/* bprotocol.c - FreeRTOS/lwip based implemenation of the Blinkenlights udp protocol
 *
 * Copyright (c) 2008  The Blinkenlights Crew
 *                          Daniel Mack <daniel@caiaq.de>
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


/* Standard includes. */
#include <stdio.h>
#include <string.h>

/* Scheduler includes. */
#include <FreeRTOS.h>
#include <board.h>
#include <task.h>
#include <semphr.h>
#include <led.h>

#include "debug_printf.h"
#include "network.h"
#include "SAM7_EMAC.h"
#include "env.h"

/* RF includes */
#include "proto.h"

/* Blinkenlights includes. */
#include "bprotocol.h"

static BRFPacket rfpkg;
xSemaphoreHandle jamSemaphore;

void
bJamThread (void *pvParameters)
{
	vSemaphoreCreateBinary(jamSemaphore);

	while (1) {
		int i, delay = (xTaskGetTickCount() / portTICK_RATE_MS) & 3;

		if (!jam_mode) {
			vTaskDelay (100 / portTICK_RATE_MS);
			continue;
		}
			
		vTaskDelay (delay / portTICK_RATE_MS);

		/* funk it. */
		memset(&rfpkg, 0, sizeof(rfpkg));
		rfpkg.cmd = RF_CMD_SET_VALUES;
		rfpkg.wmcu_id = env.e.mcu_id;
		rfpkg.mac = 0xffff; /* send to all MACs */

		xSemaphoreTake (jamSemaphore, 0);

		for (i = 0; i < RF_PAYLOAD_SIZE; i++)
			rfpkg.payload[i] = (last_lamp_val[i * 2] & 0xf)
					 | (last_lamp_val[(i * 2) + 1] << 4);
		xSemaphoreGive (jamSemaphore);

		vnRFTransmitPacket(&rfpkg);
	}
}


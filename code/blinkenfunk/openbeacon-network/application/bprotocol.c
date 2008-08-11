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

#include "network.h"
#include "SAM7_EMAC.h"

/* lwIP includes. */
#include "lwip/api.h"
#include "lwip/tcpip.h"
#include "lwip/memp.h"
#include "lwip/stats.h"
#include "lwip/dhcp.h"
#include "lwip/udp.h"
#include "netif/loopif.h"
#include "env.h"

/* Blinkenlights includes. */
#include "bprotocol.h"

#define MAX_WIDTH 100
#define MAX_HEIGHT 100
#define MAX_CHANNELS 3
#define MAX_BPP 4

static struct udp_pcb *b_pcb;
static unsigned char payload[2048];

static void b_output_line(int width, unsigned char *data)
{
	// ...
}

static int b_parse_mcu_frame(mcu_frame_header_t *header, int maxlen)
{
	int len = sizeof(*header);
	unsigned char *payload = (unsigned char *) header + sizeof(*header);

	if (len > maxlen)
		return 0;

	if (header->width >= MAX_WIDTH ||
	    header->height >= MAX_HEIGHT ||
	    header->channels > MAX_CHANNELS ||
	    header->bpp >= MAX_BPP || 
	    header->bpp == 0)
	    return 0;

	len += header->width * header->height * header->channels * (8 / header->bpp);

	if (len > maxlen)
		return 0;

	debug_printf("!valid mcu frame!\n");

	if (env.e.assigned_line == -1)
		/* frame is valid but since there was no configure packet,
		 * we have nothing to display yet */
		return 0;

	if (env.e.assigned_line > header->height)
		/* we were configured to display a line which is higher than
		 * the frame's height. be beledigt. */
		return 0;

	/* get our designated line */
	payload += header->width * header->channels * (8 / header->bpp) * env.e.assigned_line;

	/* funk it. */
	b_output_line(header->width, payload);

	return len;
}

static int b_parse_mcu_setup(mcu_setup_header_t *header, int maxlen)
{
	int len = sizeof(*header);
	
	if (len > maxlen)
		return 0;

	if (header->width >= MAX_WIDTH ||
	    header->height >= MAX_HEIGHT ||
	    header->channels > MAX_CHANNELS)
	    return 0;

	len += header->width * header->height * header->channels;

	if (len > maxlen)
		return 0;

	return len;
}

static int b_parse_mcu_devctrl(mcu_devctrl_header_t *header, int maxlen)
{
	int len = sizeof(*header);

	if (len > maxlen)
		return 0;

	debug_printf(" %s() cmd %04x\n", __func__, header->command);

	switch (header->command) {
		case MCU_DEVCTRL_COMMAND_SET_LINE:
			env.e.assigned_line = header->value;
			debug_printf("new assigned line received: %d\n", env.e.assigned_line);
			env_store();
			break;
	}

	return 0; //len;
}

static void b_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	unsigned int off = 0;

debug_printf("%s(): %d bytes\n", __func__, p->len);

	if (p->len < sizeof(unsigned int) || p->len > sizeof(payload)) {
		pbuf_free(p);
		return;
	}


	/* package payload has to be copied to a local buffer for whatever reason */
	memcpy(payload, p->payload, p->len);
	
	do {
		unsigned int magic = *(unsigned int *) payload + off;
		unsigned int consumed = 0;

		debug_printf(" magic %04x\n", magic);

		switch (magic) {
			case MAGIC_MCU_FRAME:
				consumed = b_parse_mcu_frame((mcu_frame_header_t *) payload + off, p->len - off);
				break;
			case MAGIC_MCU_SETUP:
				consumed = b_parse_mcu_setup((mcu_setup_header_t *) payload + off, p->len - off);
				break;
			case MAGIC_MCU_DEVCTRL:
				consumed = b_parse_mcu_devctrl((mcu_devctrl_header_t *) payload + off, p->len - off);
				break;
		}

		if (consumed == 0)
			break;

		off += consumed;
	} while (off < p->len);

	pbuf_free(p);
}

void bprotocol_init(void)
{
	b_pcb = udp_new();

	udp_recv(b_pcb, b_recv, NULL);
	udp_bind(b_pcb, IP_ADDR_ANY, MCU_LISTENER_PORT);
	debug_printf("%s()\n", __func__);
}


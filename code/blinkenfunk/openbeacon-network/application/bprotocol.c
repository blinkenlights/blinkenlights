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

/* lwIP includes. */
#include "lwip/ip.h"
#include "lwip/udp.h"

/* Blinkenlights includes. */
#include "bprotocol.h"

/* RF includes */
#include "proto.h"

#define MAX_WIDTH 100
#define MAX_HEIGHT 100
#define MAX_CHANNELS 3
#define MAX_BPP 4

static struct udp_pcb *b_pcb;
static unsigned char payload[2048];
static BRFPacket rfpkg;

static void b_output_line(int width, unsigned char *data)
{
	int i;

	memset(&rfpkg, 0, sizeof(rfpkg));
	rfpkg.cmd = RF_CMD_SET_VALUES;
	rfpkg.line = env.e.assigned_line;
	rfpkg.mac = 0xffff; /* send to all MACs */

	for (i = 0; (i * 2 < width) && (i < RF_PAYLOAD_SIZE); i++)
		rfpkg.payload[i] = (data[i * 2 + 0] << 4) |
				   (data[i * 2 + 1] & 0xf);

	vnRFTransmitPacket(&rfpkg);
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

static void b_set_lamp_id(int lamp_id, int lamp_line, int lamp_mac)
{
	debug_printf("lamp MAC %08x -> ID %d\n", lamp_mac, lamp_id);

	memset(&rfpkg, 0, sizeof(rfpkg));
	rfpkg.cmd = RF_CMD_SET_LAMP_ID;
	
	rfpkg.mac = lamp_mac;
	rfpkg.line = 0xff;
	rfpkg.set_lamp_id.id = lamp_id;
	rfpkg.set_lamp_id.line = lamp_line;
	vnRFTransmitPacket(&rfpkg);
}

static void b_set_gamma_curve(int lamp_mac, unsigned short *gamma)
{
	int i;

	memset(&rfpkg, 0, sizeof(rfpkg));
	rfpkg.cmd = RF_CMD_SET_GAMMA;
	rfpkg.mac = lamp_mac;

	for (i = 0; i < 8; i++)
		rfpkg.set_gamma.val[i] = gamma[i];

	vnRFTransmitPacket(&rfpkg);
}

static void b_write_gamma_curve(int lamp_mac)
{
	memset(&rfpkg, 0, sizeof(rfpkg));
	rfpkg.cmd = RF_CMD_WRITE_GAMMA;
	rfpkg.mac = lamp_mac;
	vnRFTransmitPacket(&rfpkg);
}

static void b_set_lamp_jitter(int lamp_mac, int jitter)
{
	memset(&rfpkg, 0, sizeof(rfpkg));
	rfpkg.cmd = RF_CMD_SET_JITTER;
	rfpkg.mac = lamp_mac;
	rfpkg.set_jitter.jitter = jitter;
	vnRFTransmitPacket(&rfpkg);
}

static int b_parse_mcu_devctrl(mcu_devctrl_header_t *header, int maxlen)
{
	int len = sizeof(*header);

	if (len > maxlen)
		return 0;

	debug_printf(" %s() cmd %04x\n", __func__, header->command);

	switch (LWIP_PLATFORM_HTONL(header->command)) {
		case MCU_DEVCTRL_COMMAND_SET_LINE:
			env.e.assigned_line = LWIP_PLATFORM_HTONL(header->value);
			debug_printf("new assigned line received: %d\n", env.e.assigned_line);
			env_store();
			break;
		case MCU_DEVCTRL_COMMAND_SET_LAMP_ID: {
			int lamp_mac = LWIP_PLATFORM_HTONL(header->mac);
			int lamp_id = LWIP_PLATFORM_HTONL(header->value);
			int lamp_line = LWIP_PLATFORM_HTONL(header->param[0]);
			b_set_lamp_id(lamp_id, lamp_line, lamp_mac);
			break;
		}
		case MCU_DEVCTRL_COMMAND_SET_GAMMA: {
			unsigned short i, gamma[8];
			int lamp_mac = LWIP_PLATFORM_HTONL(header->mac);
			
			for (i = 0; i < 8; i++)
				gamma[i] = LWIP_PLATFORM_HTONL(header->param[i]);
			
			b_set_gamma_curve(lamp_mac, gamma);
			break;
		}
		case MCU_DEVCTRL_COMMAND_WRITE_GAMMA: {
			int lamp_mac = LWIP_PLATFORM_HTONL(header->mac);
			b_write_gamma_curve(lamp_mac);
			break;
		}
		case MCU_DEVCTRL_COMMAND_SET_JITTER: {
			int lamp_mac = LWIP_PLATFORM_HTONL(header->mac);
			int jitter = LWIP_PLATFORM_HTONL(header->value);
			b_set_lamp_jitter(lamp_mac, jitter);
			break;
		}
	}

	return 0; //len;
}

static void b_recv(void *arg, struct udp_pcb *pcb, struct pbuf *p, struct ip_addr *addr, u16_t port)
{
	unsigned int off = 0;

	if (p->len < sizeof(unsigned int) || p->len > sizeof(payload)) {
		pbuf_free(p);
		return;
	}

	/* package payload has to be copied to a local buffer for whatever reason */
	memcpy(payload, p->payload, p->len);
	
	do {
		unsigned int magic = LWIP_PLATFORM_HTONL(*(unsigned int *) payload + off);
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
	debug_printf("%s()\n\r", __func__);
}


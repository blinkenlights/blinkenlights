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
	memset(&rfpkg, 0, sizeof(rfpkg) - RF_PAYLOAD_SIZE);
	rfpkg.cmd = RF_CMD_SET_VALUES;
	rfpkg.wmcu_id = env.e.mcu_id;
	rfpkg.mac = 0xffff; /* send to all MACs */

	//hex_dump((unsigned char *) rfpkg.payload, 0, RF_PAYLOAD_SIZE);
	/* rfpkg.payload pre-filled */
	vnRFTransmitPacket(&rfpkg);
}

static unsigned char b_mcu_frame_get_pixel_val(mcu_frame_header_t *header, int pixel, unsigned int maxlen)
{
	unsigned char *payload = (unsigned char *) header + sizeof(*header);
	unsigned int v;
	//unsigned int pos = (pixel * header->channels * header->bpp) / 8;
	unsigned int pos = (pixel * header->channels);

	if (pos >= maxlen - sizeof(*header))
		return 0;

	v = payload[pos];

/*
	if ((header->bpp == 4) && (~pixel & 1))
		v >>= 4;
*/	
	return v;
}

static int b_parse_mcu_frame(mcu_frame_header_t *header, int maxlen)
{
	int i, len = sizeof(*header);

//	if (len > maxlen)
//		return 0;

	/* HACK! */
	header->channels = 1;
	header->bpp = 4;


debug_printf(" parsing mcu frame\n");
debug_printf(" -- w %d h %d chns %d bpp %d\n",
	header->width,
	header->height,
	header->channels,
	header->bpp);

	if (header->width >= MAX_WIDTH ||
	    header->height >= MAX_HEIGHT ||
	    header->channels > MAX_CHANNELS ||
	    header->bpp > MAX_BPP || 
	    header->bpp == 0)
	    return 0;

	len += (header->width * header->height * header->channels * header->bpp) / 8;

//	if (len > maxlen)
//		return 0;

	debug_printf("!valid mcu frame! len = %d\n", len);

	if (env.e.mcu_id == -1)
		/* frame is valid but since there was no configure packet,
		 * we have nothing to display yet */
		return 0;

	/* ... */
	memset(&rfpkg.payload, 0, RF_PAYLOAD_SIZE);
	for (i = 0; i < RF_PAYLOAD_SIZE; i++) {
		rfpkg.payload[i] = (b_mcu_frame_get_pixel_val(header, env.e.lamp_map[i*2], maxlen) & 0xf)
				 | (b_mcu_frame_get_pixel_val(header, env.e.lamp_map[i*2 + 1], maxlen) << 4);
	}
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

static inline void b_set_lamp_id(int lamp_id, int lamp_mac)
{
	debug_printf("lamp MAC %08x -> ID %d\n", lamp_mac, lamp_id);

	memset(&rfpkg, 0, sizeof(rfpkg));
	rfpkg.cmd = RF_CMD_SET_LAMP_ID;
	
	rfpkg.mac = lamp_mac;
	rfpkg.wmcu_id = 0xff;
	rfpkg.set_lamp_id.id = lamp_id;
	rfpkg.set_lamp_id.wmcu_id = env.e.mcu_id;
	vnRFTransmitPacket(&rfpkg);
}

static inline void b_set_gamma_curve(int lamp_mac, unsigned int block, unsigned short *gamma)
{
	int i;

	memset(&rfpkg, 0, sizeof(rfpkg));
	rfpkg.cmd = RF_CMD_SET_GAMMA;
	rfpkg.mac = lamp_mac;
	rfpkg.set_gamma.block = block;

	for (i = 0; i < 8; i++)
		rfpkg.set_gamma.val[i] = gamma[i];

	vnRFTransmitPacket(&rfpkg);
}

static inline void b_write_gamma_curve(int lamp_mac)
{
	memset(&rfpkg, 0, sizeof(rfpkg));
	rfpkg.cmd = RF_CMD_WRITE_CONFIG;
	rfpkg.mac = lamp_mac;
	vnRFTransmitPacket(&rfpkg);
}

static inline void b_set_lamp_jitter(int lamp_mac, int jitter)
{
	memset(&rfpkg, 0, sizeof(rfpkg));
	rfpkg.cmd = RF_CMD_SET_JITTER;
	rfpkg.mac = lamp_mac;
	rfpkg.set_jitter.jitter = jitter;
	vnRFTransmitPacket(&rfpkg);
}

static inline void b_set_assigned_lamps(unsigned int *map)
{
	int i;

	for (i = 0; i < MAX_LAMPS; i++) {
		env.e.lamp_map[i] = map[i];
	}

	env_store();
	debug_printf("new assigned lamps set.\n");
}

static int b_parse_mcu_devctrl(mcu_devctrl_header_t *header, int maxlen)
{
//	int len = sizeof(*header);

//	if (len > maxlen)
//		return 0;

	debug_printf(" %s() cmd %04x\n", __func__, header->command);

	switch (header->command) {
		case MCU_DEVCTRL_COMMAND_SET_MCU_ID:
			env.e.mcu_id = header->value;
			debug_printf("new MCU ID assigned: %d\n", env.e.mcu_id);
			env_store();
			break;
		case MCU_DEVCTRL_COMMAND_SET_LAMP_ID: {
			int lamp_mac = header->mac;
			int lamp_id = header->value;
			b_set_lamp_id(lamp_id, lamp_mac);
			break;
		}
		case MCU_DEVCTRL_COMMAND_SET_GAMMA: {
			unsigned short i, gamma[8];
			int lamp_mac = header->mac;
			int block = header->value;
			
			for (i = 0; i < 8; i++)
				gamma[i] = header->param[i];
			
			b_set_gamma_curve(lamp_mac, block, gamma);
			break;
		}
		case MCU_DEVCTRL_COMMAND_WRITE_CONFIG: {
			int lamp_mac = header->mac;
			b_write_gamma_curve(lamp_mac);
			break;
		}
		case MCU_DEVCTRL_COMMAND_SET_JITTER: {
			int lamp_mac = header->mac;
			int jitter = header->value;
			b_set_lamp_jitter(lamp_mac, jitter);
			break;
		}
		case MCU_DEVCTRL_COMMAND_SET_ASSIGNED_LAMPS: {
			b_set_assigned_lamps(header->param);
			break;
		}
		case MCU_DEVCTRL_COMMAND_OUTPUT_RAW: {
			int i;

			rfpkg.cmd = header->param[0];
			rfpkg.mac = header->param[1];
			rfpkg.wmcu_id = header->param[2];
			for (i = 0; i < RF_PAYLOAD_SIZE; i++)
				rfpkg.payload[i] = header->param[i+3];
		
			hex_dump((unsigned char *) &rfpkg, 0, sizeof(rfpkg));
			vnRFTransmitPacket(&rfpkg);
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
	shuffle_tx_byteorder((unsigned long *) payload, p->len / sizeof(long));
	
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
	debug_printf("%s()\n\r", __func__);
}


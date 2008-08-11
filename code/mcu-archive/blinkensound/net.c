/*
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "values.h"
#include "net.h"

static int    *net_sockets;
static int     n_sockets;
static Packet  packet;

typedef struct _SetupPacket SetupPacket;
struct _SetupPacket {
	mcu_setup_header_t  header;
	mcu_setup_pixel_t   pixel[WIDTH * HEIGHT];
};

int net_init (const char *hosts[], int n_hosts) {
        struct hostent     *dest;
        struct sockaddr_in  addr;
#if 0 
	SetupPacket         setup;
#endif
	int  i, j;

        net_sockets = malloc (sizeof(int) * n_hosts);

        for (i = 0, j = 0; i < n_hosts; i++) {
                if (hosts[i][0] == '-')
                        continue;

                if (! (dest = gethostbyname (hosts[i]))) {
                        fprintf (stderr, "Couldn't resolve host '%s': %s\n",
                                 hosts[i], hstrerror (h_errno));
                        return -1;
                }
                net_sockets[j] = socket (PF_INET, SOCK_DGRAM, IPPROTO_UDP);
                if (net_sockets[j] < 0) {
                        perror ("socket");
                        return -1;
                }
                addr.sin_family = dest->h_addrtype;
                memcpy (&addr.sin_addr.s_addr,
                        dest->h_addr_list[0], dest->h_length);
                addr.sin_port = htons (MCU_LISTENER_PORT);

                if (connect (net_sockets[j],
                             (struct sockaddr *) &addr, sizeof (addr)) < 0) {
                        perror ("connect");
                        return -1;
                }
                j++;
        }

        n_sockets = j;

        /* prepare frame packets */
        packet.header.magic  = htonl (MAGIC_MCU_FRAME);
        packet.header.height = htons (HEIGHT);
        packet.header.width  = htons (WIDTH);
        packet.header.depth  = htons (1);
 	packet.header.maxval = htons (0xFF);

#if 0
        /* send setup packet */
        setup.header.magic  = htonl (MAGIC_MCU_SETUP);
        setup.header.mcu_id = -1;
        setup.header.height = htons (HEIGHT);
        setup.header.width  = htons (WIDTH);
        setup.header.depth  = htons (1);
        setup.header.pixels = htons (WIDTH * HEIGHT);

        /* the Nation of Gondwana installation uses this slightly
           weird setup ...
         */
        for (i = 0; i < WIDTH * HEIGHT; i ++) {
               setup.pixel[i].row    = i % WIDTH;
               setup.pixel[i].column = i / WIDTH;
        }

        for (i = 0; i < n_sockets; i++)
                send (net_sockets[i], &setup, sizeof (SetupPacket), 0);
#endif

        return i;
}

void send_packet (void) {
        int i;

	memcpy (packet.data, matrix, sizeof (packet.data));
        for (i = 0; i < n_sockets; i++)
                send (net_sockets[i], &packet, sizeof (Packet), 0);
}

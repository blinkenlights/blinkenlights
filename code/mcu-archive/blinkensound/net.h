#ifndef _NET_H_
#define _NET_H_

#include "../mcud/mcud.h"

typedef struct _Packet Packet;
struct _Packet
{
	mcu_frame_header_t header;
        u_char             data[HEIGHT][WIDTH];
};

int  net_init (const char *hosts[], int n_hosts);
void send_packet (void);

#endif /* _NET_H_ */

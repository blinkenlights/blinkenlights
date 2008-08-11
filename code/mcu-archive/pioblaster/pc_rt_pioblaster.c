#include "../windowmatrixd/bmc_pio/bmc_pio.h"

#include <linux/module.h>
#include <linux/modversions.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");


#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>

#define INTERVAL_NS  ( 1000 * 1000 * 1000 )

#define BASE_ADDRESS 0x200


// pioblaster settings

#define CHIP 0
#define PORT 0


RT_TASK blast_task;
int invocation_counter = 0;


static void blast (int data)
{
	int index = 0;

#ifdef VALUE_TABLE
	int table_size = 6;
	char value_table[] = {
		0xf8, 0x80, 0x74, 0xc8, 0xf2, 0xac, 0x50, 0xa0, 0xdc, 0xe3
	0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00, 0x01, 0x01, 0x00, 0x00
	0x07, 0x06, 0x04, 0x01, 0x02, 0x02, 0x05, 0x04, 0x06, 0x03, 0x00, 0x00
	0x03, 0x02, 0x01, 0x02, 0x03, 0x00
	};
#endif

	int bits = 32;
	int ports = 4;
	u32 bitfield [] = { 0x10000000, 0xaaaaaaaa, 0x11111111, 0x33333333 };

	char value;
	int port;

	while(1) {
		invocation_counter++;

		value = 0;
		for ( port = 0; port < ports; port++)
			value |= ( ( (bitfield[port] >> index) & 0x01) << port);

		outb( value, BASE_ADDRESS + 0);

		index++;
		if(index == bits)
			index=0;

#ifdef VALUE_TABLE
		outb( value_table[index], BASE_ADDRESS );
		if(index == table_size)
			index=0;
#endif VALUE_TABLE

		rt_task_wait_period();
	}
}

int init_module(void)
{
	RTIME period;
	int status;

	printk ("rt_pioblaster: init_module\n");

	outb( 0x80, BASE_ADDRESS + 3);
	outb( 0x55, BASE_ADDRESS + 0);

	rt_task_init(&blast_task, &blast, 0, 4096, 1, 0, NULL);

	rt_set_periodic_mode();
	period = start_rt_timer(nano2count(INTERVAL_NS));
	printk("rt_pioblaster: nano %d = count %d\n", 1000*1000*1000, (int)nano2count(1000*1000*1000));
	printk("rt_pioblaster: period = %d\n", (int) period);
	status = rt_task_make_periodic_relative_ns(&blast_task, 0, INTERVAL_NS/100);

	return 0;
}

void cleanup_module (void)
{
	rt_task_delete(&blast_task);
	outb( 0x00, BASE_ADDRESS + 0);
	printk ("rt_pioblaster: cleanup_module (%d)\n", invocation_counter);
}

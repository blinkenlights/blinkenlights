#include "../windowmatrixd/bmc_pio/bmc_pio.h"

#include <linux/module.h>
//#include <linux/modversions.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");


#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>

#include "sc410.h"

#define TIMER_PERIOD_NS		( 1000 * 1000 * 100 )
#define TASK_PERIOD  		100

#define BASE_ADDRESS 0x200


// pioblaster settings

#define CHIP 0
#define PORT 0


/////////////////////////////////////////////////////////////////////////
// SC410 Low Level Function: windex -- Write Byte to CSC Registerspace

static void windex (unsigned char index, unsigned char data)
{
	outb (index, SC410_CSCIR);
	outb (data, SC410_CSCDP);
}

///////////////////////////////////////////////////////////////////////////
// SC410 Low Level Function: rindex -- Read Byte from CSC Registerspace
static unsigned char rindex (unsigned char index)
{
	outb (index, SC410_CSCIR);
	return (inb (SC410_CSCDP));
}



RT_TASK blast_task;
int invocation_counter = 0;


static void bitblaster (int data)
{
	int index = 0;

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

		rt_task_wait_period();
	}
}


#define GREYSCALER_STEP_NS 	(1000 * 100)
#define GREYSCALER_MAX_DELAY_NS 	(1000 * 1000 * 9)

static void greyscaler (int data)
{
	static int delay_ns = GREYSCALER_MAX_DELAY_NS;
	int delay_counter = 0;

	outb( 0x00, BASE_ADDRESS + 0);

	while (1) {

		rt_sleep( nano2count(delay_ns) );
		outb( 0xff, BASE_ADDRESS + 0);

		rt_sleep( nano2count(1000 * 10) );
		outb( 0x00, BASE_ADDRESS + 0);

		delay_counter += 1;
		if(delay_counter >= 10) {
			delay_counter = 0;
			delay_ns -= GREYSCALER_STEP_NS;
		}

		if ( delay_ns <= 0 ) {
			delay_ns = GREYSCALER_MAX_DELAY_NS;
		}

		rt_task_wait_period();
	}
}

int init_module(void)
{
	RTIME period;
	int status;

	printk ("rt_pioblaster: init_module\n");


	// Set DNP/1486 CS1 for I/O address space 0x200 - 0x207
	// Set DNP/1486 CS2 for I/O address space 0x280 - 0x28f
	// ====================================================

	windex (SC410_GPIO_RBWR_A, rindex (SC410_GPIO_RBWR_A) | 0x03); // Step 1
	windex (SC410_GPIO_CS_FSR_A, rindex (SC410_GPIO_CS_FSR_A) | 0x05); // Step 2
	windex (SC410_GPIO_TCR_A, rindex (SC410_GPIO_TCR_A)| 0x03); // Step 3
	windex (SC410_SMPSOR, (rindex (SC410_SMPSOR) & 0xfe) | 0x01); // Step 4
	windex (SC410_GP_CSA_IO_ADR, 0x00); // CSA Step 5.1

	windex (SC410_GP_CSA_IO_ADMR, 0x22); // CSA Step 5.2
	windex (SC410_GP_CSB_IO_ADR, 0x80); // CSB Step 5.1
	windex (SC410_GP_CSB_IO_ADMR, 0x02); // CSB Step 5.2
	windex (SC410_GP_CSAB_IO_CQR, (rindex (SC410_GP_CSAB_IO_CQR) & 0x88) | 0x33); // Step 6
	windex (SC410_GP_CS_GPIO_CS_MR_A_IO_ADR, 0x10); // Step 7
	windex (SC410_GPIO_RBWR_A, rindex (SC410_GPIO_RBWR_A) & 0xfc); // Step 8


	outb( 0x80, BASE_ADDRESS + 3);
//	outb( 0x55, BASE_ADDRESS + 0);

//	rt_task_init(&blast_task, &bitblaster, 0, 4096, 1, 0, NULL);
	rt_task_init(&blast_task, &greyscaler, 0, 4096, 1, 0, NULL);

	rt_set_periodic_mode();
	period = start_rt_timer(nano2count(TIMER_PERIOD_NS));
//	period = start_rt_timer(INTERVAL_COUNT);
	printk("rt_pioblaster: nano %d = count %d\n\n", 1000*1000*1000, (int)nano2count(1000*1000*1000));

	printk("rt_pioblaster: timer period = %d\n", (int) period);
	status = rt_task_make_periodic_relative_ns(&blast_task, 0, TASK_PERIOD);

	return 0;
}

void cleanup_module (void)
{
	rt_task_delete(&blast_task);
	outb( 0x00, BASE_ADDRESS + 0);
	printk ("rt_pioblaster: cleanup_module (%d)\n", invocation_counter);
}

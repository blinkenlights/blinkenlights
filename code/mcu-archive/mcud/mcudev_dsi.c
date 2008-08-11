/*
 * mcudev_dsi
 *
 * MCU Driver Module for fluorescent tubes with DSI interface.
 */

#include <linux/module.h>
#include <asm/io.h>

MODULE_AUTHOR("Tim Pritlove");
MODULE_DESCRIPTION("MCU Driver Module for fluorescent tubes with DSI interface");
MODULE_LICENSE("GPL");

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>
#include <rtai_shm.h>

#include "mcu.h"
#include "mcud.h"

#define MCUDEV_NAME             "mcu_dsi"

#define UPDATE_PERIOD_NS		833333		// 833,333 us = 1200 ticks / second
#define MCU_PIXELS              8

/*
 * globals
 */

RT_TASK mcudev_dsi_task;
mcu_device_t * Device;

/*
 * DSI Protocol states
 */

unsigned char dsi_frame_data[MCU_PIXELS][16];

typedef enum dsi_state {
	DSI_STATE_PAUSE,
	DSI_STATE_START,
	DSI_STATE_DATA,
	DSI_STATE_END
} dsi_state_t, DSI_STATE;

struct dsi_state_info {
	dsi_state_t state;
	dsi_state_t next_state;
	int ticks;
	int static_data;
	unsigned char port_data;
};


/*
 * encode_dsi_byte
 *
 * manchester encode a byte to unsigned char array
 * in MSB to LSB to order
 *
 * [ 0, 1]: manchester encoded bit 0
 * [ 2, 3]: manchester encoded bit 1
 * [ 4, 5]: manchester encoded bit 2
 * [ 6, 7]: manchester encoded bit 3
 * [ 8, 9]: manchester encoded bit 4
 * [10,11]: manchester encoded bit 4
 * [12,13]: manchester encoded bit 5
 * [14,15]: manchester encoded bit 7
 */

static void
encode_dsi_byte (unsigned char value, unsigned char dsi_byte[])
{
	int index;

	for (index = 0;index < 8;index++) {
		if( (value & (1 << (7 - index))) == 0) {
			// logical ZERO: 0 -> 1 transition
			dsi_byte[2*index+0] = 0;
			dsi_byte[2*index+1] = 1;
		} else {
			// logical ONE: 1 -> 0 transition
			dsi_byte[2*index+0] = 1;
			dsi_byte[2*index+1] = 0;
		}
	}
}


/*
 * dsi_blaster
 *
 * realtime function being timed on 1 dsi clock tick (833us,
 * 1200 times/second). The function holds a current state
 * and a current tick per state.
 */

#define DSI_0      1,0        // left to right
#define DSI_1      0,1        // left to right

static void
mcudev_dsi_process (int data)
{
	struct dsi_state_info dsi_state [] = {
		{ DSI_STATE_PAUSE, DSI_STATE_START, 79, TRUE, 0 },
		{ DSI_STATE_START, DSI_STATE_DATA,   1, TRUE, 1 },
		{  DSI_STATE_DATA, DSI_STATE_END,   16, FALSE },
		{   DSI_STATE_END, DSI_STATE_PAUSE,  4, TRUE, 0 }
	};

	DSI_STATE dsi_current_state = DSI_STATE_PAUSE;
	int dsi_current_tick = 0;

	unsigned char port_data, value;
	int bit, port, pio_port;
	
	while (TRUE) {
		// check for end of state
		if (dsi_current_tick == dsi_state[dsi_current_state].ticks) {
			// move to next state, reset tick count
			dsi_current_state = dsi_state[dsi_current_state].next_state;
			dsi_current_tick = 0;

			if (dsi_current_state == DSI_STATE_PAUSE) {
				// set up dsi_frame_data array with current data
				for (port = 0;port < MCU_PIXELS;port++) {
					encode_dsi_byte(Device->pixel[port][0], &dsi_frame_data[port][0]);
				}
			}
		}

		/*
		 * Collect port data to be written and write 8 bits
		 * a time for performance reasons.
		 */
		bit = 0, value = 0, pio_port = 0;
		for(port = 0; port < MCU_PIXELS; port++) {
			if (dsi_state[dsi_current_state].static_data == TRUE) {
				port_data = dsi_state[dsi_current_state].port_data;
			} else {
				port_data = dsi_frame_data[port][dsi_current_tick];
			}
	
			value |= ( (port_data & 0x01) << bit);
			bit++;

			if(bit == 8) {
				outb( value, pio_port_io_addr[pio_port]);
				bit = 0;
				pio_port++;
				value = 0;
			}
		}

		// increase tick
		dsi_current_tick += 1;

		rt_task_wait_period();
	}
}


int
init_module(void)
{
	RTIME period, start_time;
	int port;
	int status;

	printk ("%s: init_module\n", MCUDEV_NAME);

	Device = (mcu_device_t *) rtai_kmalloc(MCU_DEVICE_SHMEM_MAGIC, sizeof(mcu_device_t));
	if (Device == NULL) {
		printk ("%s: rtai_kmalloc failed\n", MCUDEV_NAME);
		return 1;
	}
	
	strcpy(Device->name, MCUDEV_NAME);
	Device->depth = 1;
	Device->maxval = 255;
	Device->flag_zero_off = 1;
	for(port = 0;port < MCU_PIXELS;port++)
		Device->pixel[port][0] = 0;

	mcu_setup_pio();

	status = rt_task_init(&mcudev_dsi_task, &mcudev_dsi_process, 0, 4096, 1, 0, NULL);
	if(status != 0) {
		printk("%s: rt_task_init failed (%d)\n", MCUDEV_NAME, status);
		rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);
		return 1;
	}

	rt_set_periodic_mode();
	period = start_rt_timer(nano2count(UPDATE_PERIOD_NS));
	start_time = rt_get_time() + 10 * period;
	status = rt_task_make_periodic(&mcudev_dsi_task, start_time, period);
	if(status != 0) {
		printk("%s: rt_task_make_periodic failed (%d)\n", MCUDEV_NAME, status);
		rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);
		return 1;
	}

	printk ("%s: mcudev_dsi task started\n", MCUDEV_NAME);

	Device->flag_loaded = TRUE;
	return 0;
}

void
cleanup_module (void)
{
	Device->flag_loaded = FALSE;
	rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);

	rt_task_delete(&mcudev_dsi_task);

	printk ("%s: module unloaded\n", MCUDEV_NAME);
}

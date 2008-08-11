/*
 * mcudev_rgb
 *
 * MCU Driver Module for DIMMBOX controlled RGB neon tubes
 */

#include <linux/module.h>
#include <asm/io.h>

MODULE_AUTHOR("Tim Pritlove");
MODULE_DESCRIPTION("MCU Driver Module for for DIMMBOX controlled RGB neon tubes");
MODULE_LICENSE("GPL");

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>
#include <rtai_shm.h>

#include "mcu.h"
#include "mcud.h"

/*
 * program options
 */
#define INVERT_OUTPUT

#define MCUDEV_NAME             "mcu_rgb"

#define UPDATE_PERIOD_NS		60000		// 60 us
#define MCU_PIXELS              48


typedef enum rgb_state {
	RGB_STATE_PAUSE,
	RGB_STATE_GREEN,
	RGB_STATE_RED,
	RGB_STATE_BLUE
} rgb_state_t, RGB_STATE;

typedef struct rgb_state_info {
	rgb_state_t state;
	rgb_state_t next_state;
	int ticks;
	int static_data;
	unsigned char port_data;
	unsigned char rgb_frame_data[MCU_PIXELS][40];
} rgb_state_info_t;

/*
 * Globals
 */

RT_TASK mcudev_rgb_task;
mcu_device_t * Device;

/*
 * encode_rgb_byte
 *
 * encode a byte to int array according to RGB protocol
 * LSB to MSB order
 */

static void
encode_rgb_byte (unsigned char value, unsigned char frame_data[])
{
	int index;

	for (index = 0;index < 8;index++) {
		frame_data[5*index+0] = 1;
		if( (value & (1 << index)) == 0) {
			// logical ZERO: 0 -> 1 transition
			frame_data[5*index+1] = 0;
			frame_data[5*index+2] = 0;
			frame_data[5*index+3] = 0;
		} else {
			// logical ONE: 1 -> 0 transition
			frame_data[5*index+1] = 1;
			frame_data[5*index+2] = 1;
			frame_data[5*index+3] = 1;
		}
		frame_data[5*index+4] = 0;
	}
}



/*
 * mcudev_rgb_process
 *
 * realtime function being timed on 1 rgb clock tick (60us)
 * The function holds a current state
 * and a current tick per state.
 */


#define RGB_0      1,0,0,0,0        // left to right
#define RGB_1      1,1,1,1,0        // left to right

#define RGB_RED   0
#define RGB_GREEN 1
#define RGB_BLUE  2


static void
mcudev_rgb_process (int data)
{
	struct rgb_state_info rgb_state [] = {
		{ RGB_STATE_PAUSE, RGB_STATE_GREEN, 50, TRUE, 0 },
		{ RGB_STATE_GREEN, RGB_STATE_RED,   40, FALSE },
		{ RGB_STATE_RED,   RGB_STATE_BLUE,  40, FALSE },
		{ RGB_STATE_BLUE,  RGB_STATE_PAUSE, 40, FALSE },
	};

	RGB_STATE rgb_current_state = RGB_STATE_PAUSE;
	int rgb_current_tick = 0;

	unsigned char port_data, value;
	int bit, port, pio_port;
	
	while (TRUE) {
		// check for end of state
		if (rgb_current_tick == rgb_state[rgb_current_state].ticks) {
			// move to next state, reset tick count
			rgb_current_state = rgb_state[rgb_current_state].next_state;
			rgb_current_tick = 0;

			if (rgb_current_state == RGB_STATE_PAUSE) {
				// set up frame_data array with current data
				for (port = 0;port < MCU_PIXELS;port++) {
					encode_rgb_byte(Device->pixel[port][RGB_RED],   &rgb_state[RGB_STATE_RED].rgb_frame_data[port][0]);
					encode_rgb_byte(Device->pixel[port][RGB_GREEN], &rgb_state[RGB_STATE_GREEN].rgb_frame_data[port][0]);
					encode_rgb_byte(Device->pixel[port][RGB_BLUE],  &rgb_state[RGB_STATE_BLUE].rgb_frame_data[port][0]);
				}
			}
		}

		/*
		 * Collect port data to be written and write 8 bits
		 * a time for performance reasons.
		 */
		bit = 0, value = 0, pio_port = 0;
		for(port = 0; port < MCU_PIXELS; port++) {
			if (rgb_state[rgb_current_state].static_data == TRUE) {
				port_data = rgb_state[rgb_current_state].port_data;
			} else {
				port_data = rgb_state[rgb_current_state].rgb_frame_data[port][rgb_current_tick];
			}

#ifdef INVERT_OUTPUT
			port_data = (port_data ^ 0x01);
#endif

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
		rgb_current_tick += 1;

		rt_task_wait_period();
	}
}


int
init_module(void)
{
	RTIME period, start_time;
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

	mcu_setup_pio();

	status = rt_task_init(&mcudev_rgb_task, &mcudev_rgb_process, 0, 4096, 1, 0, NULL);
	if(status != 0) {
		printk("%s: rt_task_init failed (%d)\n", MCUDEV_NAME, status);
		rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);
		return 1;
	}

	rt_set_periodic_mode();
	period = start_rt_timer(nano2count(UPDATE_PERIOD_NS));
	start_time = rt_get_time() + 10 * period;
	status = rt_task_make_periodic(&mcudev_rgb_task, start_time, period);
	if(status != 0) {
		printk("%s: rt_task_make_periodic failed (%d)\n", MCUDEV_NAME, status);
		rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);
		return 1;
	}

	printk ("%s: mcudev_rgb task started\n", MCUDEV_NAME);

	Device->flag_loaded = TRUE;
	return 0;
}

void
cleanup_module (void)
{
	Device->flag_loaded = FALSE;
	rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);

	rt_task_delete(&mcudev_rgb_task);

	printk ("%s: module unloaded\n", MCUDEV_NAME);
}

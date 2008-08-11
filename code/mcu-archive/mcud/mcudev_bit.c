/*
 * mcudev_bit
 *
 * MCU Driver Module for lightbulbs.
 */

#include <linux/module.h>
#include <asm/io.h>

MODULE_AUTHOR("Tim Pritlove");
MODULE_DESCRIPTION("MCU Driver Module for lamps in monochrome mode");
MODULE_LICENSE("GPL");

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>
#include <rtai_shm.h>

#include "mcu.h"
#include "mcud.h"

#define UPDATE_PERIOD_NS		100000000         // 10ms
#define MCU_PIXELS              48

#define MCUDEV_NAME             "mcu_bit"

RT_TASK mcudev_bit_task;
mcu_device_t * Device;

static void
mcudev_bit_process (int data)
{
	unsigned char value;
	int pixel, bit, port;
	
	while(TRUE) {

		/*
		 * Collect port data to be written and write 8 bits
		 * a time for performance reasons.
		 */
		bit = 0, value = 0, port = 0;
		for(pixel = 0; pixel < MCU_PIXELS; pixel++) {
			if (Device->pixel[pixel][0] > 1) {
				rt_printk("%s: Illegal pixel value (pixel=%d value=%d)\n", MCUDEV_NAME, pixel, Device->pixel[pixel][0]);
				continue;
			}

			value |= ( (Device->pixel[pixel][0] & 0x01) << bit);
			bit++;

			if(bit == 8) {
				outb( value, (int) pio_port_io_addr[port]);
				bit = 0;
				port++;
				value = 0;
			}
		}

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
	
	strcpy(Device->name, "mcudev_bit");
	Device->depth = 1;
	Device->maxval = 1;
	Device->flag_zero_off = FALSE;

	mcu_setup_pio();

	status = rt_task_init(&mcudev_bit_task, &mcudev_bit_process, 0, 4096, 1, 0, NULL);
	if(status != 0) {
		printk("%s: rt_task_init failed (%d)\n", MCUDEV_NAME, status);
		rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);
		return 1;
	}

	rt_set_periodic_mode();
	period = start_rt_timer(nano2count(UPDATE_PERIOD_NS));
	start_time = rt_get_time() + 10 * period;
	status = rt_task_make_periodic(&mcudev_bit_task, start_time, period);
	if(status != 0) {
		printk("%s: rt_task_make_periodic failed (%d)\n", MCUDEV_NAME, status);
		rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);
		return 1;
	}

	printk ("%s: mcudev_bit task started\n", MCUDEV_NAME);

	Device->flag_loaded = TRUE;
	return 0;
}

void
cleanup_module (void)
{
	Device->flag_loaded = FALSE;
	rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);

	rt_task_delete(&mcudev_bit_task);

	printk ("%s: cleanup_module\n", MCUDEV_NAME);
}

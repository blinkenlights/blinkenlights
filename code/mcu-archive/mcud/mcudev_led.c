/*
 * mcudev_led
 *
 * MCU Driver Module for matrix-based monochrome LED displays
 */


#include <linux/module.h>
#include <asm/io.h>

MODULE_AUTHOR("Tim Pritlove");
MODULE_DESCRIPTION("MCU Driver Module for matrix-based monochrome LED displays");
MODULE_LICENSE("GPL");

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>
#include <rtai_shm.h>

#include "mcu.h"
#include "mcud.h"

#define UPDATE_PERIOD_NS		400000         // 40ms / 100 = 400us
#define MCU_PIXELS              48


RT_TASK mcudev_led_task;
mcu_device_t * Device;

static void
mcudev_led_process (int data)
{
	unsigned char value;
	int port, bit, pio_port;
	
	while(TRUE) {

		/*
		 * Collect port data to be written and write 8 bits
		 * a time for performance reasons.
		 */
		bit = 0, value = 0, pio_port = 0;
		for(port = 0; port < MCU_PIXELS; port++) {
			value |= ( (Device->pixel[port][0] & 0x01) << bit);
			bit++;

			if(bit == 8) {
				outb( value, (int) pio_port_io_addr[pio_port]);
				bit = 0;
				pio_port++;
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

	printk ("mcudev_led: init_module\n");

	Device = (mcu_device_t *) rtai_kmalloc(MCU_DEVICE_SHMEM_MAGIC, sizeof(mcu_device_t));
	if (Device == NULL) {
		printk ("mcudev_led: rtai_kmalloc failed\n");
		return 1;
	}
	
	strcpy(Device->name, "mcudev_led");
	Device->depth = 1;
	Device->maxval = 1;
	Device->flag_zero_off = FALSE;

	mcu_setup_pio();

	status = rt_task_init(&mcudev_led_task, &mcudev_led_process, 0, 4096, 1, 0, NULL);
	if(status != 0) {
		printk("mcudev_led: rt_task_init failed (%d)\n", status);
		rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);
		return 1;
	}

	rt_set_periodic_mode();
	period = start_rt_timer(nano2count(UPDATE_PERIOD_NS));
	start_time = rt_get_time() + 10 * period;
	status = rt_task_make_periodic(&mcudev_led_task, start_time, period);
	if(status != 0) {
		printk("mcudev_led: rt_task_make_periodic failed (%d)\n", status);
		rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);
		return 1;
	}

	printk ("mcudev_led: mcudev_led task started\n");

	Device->flag_loaded = TRUE;
	return 0;
}

void
cleanup_module (void)
{
	Device->flag_loaded = FALSE;
	rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);

	rt_task_delete(&mcudev_led_task);

	printk ("mcudev_led: cleanup_module\n");
}

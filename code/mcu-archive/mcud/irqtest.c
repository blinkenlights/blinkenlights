#include <linux/module.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>
#include <rtai_shm.h>

#include "mcu.h"
#include "mcud.h"

#define UPDATE_PERIOD_NS		1000000000         // 1ms

#define MCU_PIXELS              48

#define DIM_RESOLUTION          16

#define PHASE_IRQ               3

#define IRQ_SIMULATOR

RT_TASK mcudev_dim_task;
mcu_device_t * Device;

static unsigned long dim_invocation_counter = 0;
static unsigned long irq_invocation_counter = 0;



/*
 * Real Time Process
 */

static void
mcudev_dim_process (int data)
{
	while(TRUE) {
		dim_invocation_counter += 1;
#ifdef IRQ_SIMULATOR
		rt_global_cli();
		// set bit 0 of Port A to 1
		csc_write(GPIO_RBWR_D, 0x01);
		rt_global_sti();
#endif
		rt_printk("*");

		rt_task_wait_period();
	}
}



/*
 * Interrupt Handler
 */

void phase_isr(void)
{
	rt_global_cli();

//	rt_ack_irq(PHASE_IRQ);

	rt_printk(".");
	irq_invocation_counter += 1;

#ifdef IRQ_SIMULATOR
		// set bit 0 of Port A to 0
		csc_write(GPIO_RBWR_D, 0x00);
#endif
	rt_enable_irq(PHASE_IRQ);

	rt_global_sti();		
}


int
init_module(void)
{
	RTIME period, start_time;
	int status;
	unsigned long startup;


	printk ("mcudev_mcudev_dim: init_module\n");

	Device = (mcu_device_t *) rtai_kmalloc(MCU_DEVICE_SHMEM_MAGIC, sizeof(mcu_device_t));
	if (Device == NULL) {
		printk ("mcudev_mcudev_dim: rtai_kmalloc failed\n");
		return 1;
	}
	
	strcpy(Device->name, "mcudev_dim");
	Device->depth = 1;
	Device->maxval = DIM_RESOLUTION;
	Device->flag_zero_off = FALSE;

	mcu_setup_pio();

	/*
	 * Set DIL/NetPC INT1 (SC410 PIRQ3) to IRQ PHASE_IRQ
	 */
	csc_write(ICR_B, (csc_read (ICR_B) & PIRQ2S) | (PHASE_IRQ << 4));

#ifdef IRQ_SIMULATOR
	// configure Port A/Bit 0 (GPIO24) to output, all others to input
	csc_write(GPIO_FSR_F, GPIO24_DIR);

	// set bit 0 of Port A to 0 
	csc_write(GPIO_RBWR_D, 0x00);

#endif

	/*
	 * Set up interrupt handler
	 */

	rt_global_cli();
	status = rt_request_global_irq(PHASE_IRQ, phase_isr);
	if(status != 0) {
		printk("irq: rt_request_global_irq failed\n");
		return 1;
	}
	startup = rt_startup_irq(PHASE_IRQ);
	printk("irq: rt_startup_irq returned 0x%04lx\n", startup);
	rt_enable_irq(PHASE_IRQ);
	rt_global_sti();

	/*
	 * Start real time process
	 */
	status = rt_task_init(&mcudev_dim_task, &mcudev_dim_process, 0, 4096, 1, 0, NULL);
	if(status != 0) {
		printk("mcudev_mcudev_dim: rt_task_init failed (%d)\n", status);
		rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);
		return 1;
	}

	rt_set_periodic_mode();
	period = start_rt_timer(nano2count(UPDATE_PERIOD_NS));
	start_time = rt_get_time() + period;
	status = rt_task_make_periodic(&mcudev_dim_task, start_time, period);
	if(status != 0) {
		printk("mcudev_mcudev_dim: rt_task_make_periodic failed (%d)\n", status);
		rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);
		return 1;
	}

	printk ("mcudev_mcudev_dim: mcudev_dim task started\n");

	Device->flag_loaded = TRUE;
	return 0;
}


void
cleanup_module (void)
{
	Device->flag_loaded = FALSE;
	rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);

	/*
	 * Disable IRQ
	 */
	rt_disable_irq(PHASE_IRQ);
	rt_free_global_irq(PHASE_IRQ);

	/*
	 * Terminate Real Time Process
	 */
	rt_task_delete(&mcudev_dim_task);

	printk ("mcudev_mcudev_dim: cleanup_module (%ld,%ld)\n",
		dim_invocation_counter, irq_invocation_counter);

	return;
}

/*
 * dimtest.c
 */

#include <linux/module.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>
#include <rtai_shm.h>

#include "mcu.h"
#include "mcud.h"

#define MCU_PIXELS              48

#define PHASE_IRQ               15

#define IRQ_DELAY_NS            9400000            // 950us
#define PULSE_LENGTH_NS         20000              // 50us

RT_TASK mcudev_dim_task;

int irq_delay_ns = IRQ_DELAY_NS;
MODULE_PARM (irq_delay_ns, "1i");

RTIME irq_delay = 0;        // fixed IRQ delay time
RTIME phase_length;

#define PHASE_TABLE_SIZE        100

RTIME phases[PHASE_TABLE_SIZE];

/*
 * Dimmer tables
 */

#define DIM_RESOLUTION      16

#define MIN_STEP_LENGTH_NS  10000           // 10 us minimum step time to prevent underruns

typedef struct dim_interval {
	RTIME next_step;						// time difference to next dimmer step
	unsigned char port_value[MCU_PORTS];    // data to write to ports at each interval
	RTIME last_time;						// last time this step was invoked
} dimtable_t;

long dimtable_16_ns[DIM_RESOLUTION+1] = {
	 1663000,				// 15
	  717000,				// 14
	  572000,				// 13
	  503000,				// 12
	  463000,				// 11
	  441000,				// 10
	  429000,				//  9
	  424000,				//  8
	  429000,				//  7
	  441000,				//  6
	  463000,				//  5
	  503000,				//  4
	  572000,				//  3
	  717000,				//  2
	 1663000				//  1
};


static unsigned long dim_invocation_counter = 0;
static unsigned long irq_invocation_counter = 0;



/*
 * Dimmer Process
 */

static void
mcudev_dim_process (int data)
{
	static int dim_phase_index = 0;
	RTIME pulse_length = nano2count(PULSE_LENGTH_NS);

	while(TRUE) {
//		rt_printk("*");
		rt_sleep_until(phases[dim_phase_index]);
		outb( 0x55, pio_port_io_addr[0]);

		dim_phase_index += 1;
		if(dim_phase_index == PHASE_TABLE_SIZE)
			dim_phase_index = 0;

		rt_sleep(pulse_length);
		outb( 0x00, pio_port_io_addr[0]);
	}
}


/*
 * Interrupt Handler
 */

void phase_isr(void)
{
	int status;
	static int irq_phase_index = 0;
	static int dim_process_started = 0;
	static RTIME last_irq_activation_time = 0;
	static RTIME last_phase_length = 0;
	RTIME irq_activation_time;

#ifdef PHASE_DEBUG
	static RTIME max_phase_length = 0;
	static RTIME min_phase_length = 0;
#endif

//	rt_printk(".");

	irq_delay = nano2count(irq_delay_ns);
	irq_activation_time = rt_get_time();

	if(last_irq_activation_time == 0) {
		last_irq_activation_time = irq_activation_time;
		rt_enable_irq(PHASE_IRQ);
		return;
	}

	phase_length = (irq_activation_time - last_irq_activation_time) / 2;

#ifdef PHASE_DEBUG
    if(last_phase_length != phase_length) {
		rt_printk("phase_length: %ld ns -> %ld ns\n", (unsigned long) count2nano(last_phase_length), (unsigned long) count2nano(phase_length));
	}
    if(phase_length > max_phase_length) {
		max_phase_length = phase_length;
		rt_printk("min/max: %ld ns / %ld ns\n", (unsigned long) count2nano(min_phase_length), (unsigned long) count2nano(max_phase_length));
	}
    if(phase_length < min_phase_length || min_phase_length == 0) {
		min_phase_length = phase_length;
		rt_printk("min/max: %ld ns / %ld ns\n", (unsigned long) count2nano(min_phase_length), (unsigned long) count2nano(max_phase_length));
	}
#endif

	phases[irq_phase_index] = irq_activation_time + irq_delay + (phase_length * 4);
	phases[irq_phase_index+1] = phases[irq_phase_index] + phase_length;

	irq_phase_index += 2;
	if(irq_phase_index == PHASE_TABLE_SIZE)
		irq_phase_index = 0;

	if(!dim_process_started) {
		status = rt_task_resume(&mcudev_dim_task);
		if(status != 0) {
			printk("?");
		} else {
			dim_process_started = 1;
			rt_printk("!");
		}
	}

	last_irq_activation_time = irq_activation_time;
	last_phase_length = phase_length;

#ifdef PHASE_DEBUG
	irq_invocation_counter += 1;

	if(irq_invocation_counter % 500 == 0) {
		min_phase_length = 0;
		max_phase_length = 0;
	}
#endif

	rt_enable_irq(PHASE_IRQ);
}

void
release_data(void)
{
	rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);
}


int
init_module(void)
{
	int status;
	unsigned long startup;


	printk ("dimtest: init_module\n");

	mcu_setup_pio();

	irq_delay = nano2count(irq_delay_ns);
	printk("dimtest: irq_delay_ns=%lld\n", count2nano(irq_delay) );

	/*
	 * Set DIL/NetPC INT1 (SC410 PIRQ3) to IRQ PHASE_IRQ
	 */
	csc_write(ICR_B, (csc_read (ICR_B) & PIRQ2S) | (PHASE_IRQ << 4));

	/*
	 * Set up interrupt handler
	 */

	rt_global_cli();
	status = rt_request_global_irq(PHASE_IRQ, phase_isr);
	if(status != 0) {
		printk("dimtest: rt_request_global_irq failed\n");
		return 1;
	}
	startup = rt_startup_irq(PHASE_IRQ);
	printk("dimtest: rt_startup_irq returned 0x%04lx\n", startup);
	rt_enable_irq(PHASE_IRQ);
	rt_global_sti();

	/*
	 * Start real time process
	 */
	status = rt_task_init(&mcudev_dim_task, &mcudev_dim_process, 0, 4096, 1, 0, NULL);
	if(status != 0) {
		printk("dimtest: rt_task_init failed (%d)\n", status);
		release_data();
		return 1;
	}

	rt_set_oneshot_mode();
	start_rt_timer(0);

	printk ("dimtest: mcudev_dim task started\n");

	return 0;
}


void
cleanup_module (void)
{
	release_data();

	/*
	 * Disable IRQ
	 */
	rt_disable_irq(PHASE_IRQ);
	rt_free_global_irq(PHASE_IRQ);

	/*
	 * Terminate Real Time Process
	 */
	rt_task_delete(&mcudev_dim_task);

	printk ("dimtest: cleanup_module (%ld,%ld)\n",
		dim_invocation_counter, irq_invocation_counter);

	return;
}

/*
 * mcudev_dim
 *
 * MCU Driver Module for dimmed lightbulbs.
 *
 * This driver needs self-calibrating timing. So it is using the
 * oneshot mode of RTAI because no periodic behaviour can be
 * be achieved.
 *
 * The interrupt service routing reacts to a 50Hz interrupt driven
 * by the phase change of the alternating current driving the lights.
 * The ISR readjusts the phase_length continuously and that way
 * provides the necessary parameters for the light-controlling
 * real-time process to synchronize with the phase.
 */

#include <linux/module.h>
#include <asm/io.h>

MODULE_AUTHOR("Tim Pritlove");
MODULE_DESCRIPTION("MCU Driver Module for dimmed lightbulbs using oneshot timing");
MODULE_LICENSE("GPL");

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>
#include <rtai_shm.h>

#include "mcu.h"
#include "mcud.h"

#define MCUDEV_NAME             "mcu_dim"

#define MCU_PIXELS              48                 // number of pixels supported

#define PHASE_IRQ               15                 // IRQ to be triggered by phase change


/*
 * module parameters
 */

#define IRQ_DELAY_NS            580000            // time to wait until next phase starts after IRQ occurred
int irq_delay_ns = IRQ_DELAY_NS;
MODULE_PARM (irq_delay_ns, "1i");

#define PULSE_LENGTH_NS         50000             // 23us
int pulse_length_ns = PULSE_LENGTH_NS;
MODULE_PARM (pulse_length_ns, "1i");

int init_value = 0;
MODULE_PARM (init_value, "1i");

char * phases = "000000000000000000000000000000000000000000000000000";
MODULE_PARM (phases, "s");

/*
 * globals
 */

typedef enum phase {
	PHASE_R,
	PHASE_S,
	PHASE_T
} phase_t;

static phase_t pixel_phase[MCU_PIXELS];                   // electric phase per pixel (0,1,2)

static RT_TASK mcudev_dim_task;
static mcu_device_t * Device;

static RTIME irq_delay;                                   // IRQ delay time
static RTIME pulse_length;                                // minimum relais impulse time
static RTIME phase_length;                                // current phase length


#define PHASE_TABLE_SIZE        16                 // must be a power of 2!!
static RTIME phase_table[PHASE_TABLE_SIZE];


/*
 * Dimmer tables
 */

#define DIM_RESOLUTION      16
#define DIM_TABLE           dim_table_16_ns

#define DIM_INTERVALS       (DIM_RESOLUTION-1)

static long dim_table_16_ns[16] = {
	       0,				// 15
	 1646000,				// 14
	  710000,				// 13
	  566000,				// 12
	  498000,				// 11
	  459000,				// 10
	  436000,				//  9
	  425000,				//  8
	  420000,				//  7
	  425000,				//  6
	  436000,				//  5
	  459000,				//  4
	  498000,				//  3
	  566000,				//  2
	  710000,				//  1
	 1646000				//  0 (unused, replaced by pulse_length_ns)
};



/*
 * Dimmer interval structure
 */

typedef struct dim_interval {
	RTIME time_offset;                            // cumulative time difference to this step
	unsigned char port_value[MCU_PORTS];          // data to write to ports at each interval
} dim_interval_t;

static dim_interval_t dim_interval[DIM_INTERVALS];


/*
 * setup interval table
 *
 * initialize the constant values of the table
 */

static void
setup_dim_interval()
{
	int interval, port;
	RTIME time_offset = 0;

	/*
	 * accumulate time_offsets
	 */
	for(interval = 0;interval < DIM_INTERVALS-1;interval++) {
		time_offset += nano2count(DIM_TABLE[interval]);
		dim_interval[interval].time_offset = time_offset;
	}

	/*
	 * set final interval packet to all zero to prevent
	 * the semiconductor relais to switch on the light in the next phase.
	 */
	dim_interval[DIM_INTERVALS-1].time_offset = time_offset + pulse_length;

	for(port = 0;port < MCU_PORTS;port++) {
		dim_interval[DIM_INTERVALS-1].port_value[port] = 0;
	}
}

/*
 * prepare device data
 *
 * read in device values and prepare chip-oriented data aggregation
 * producing an array of values that can be written directly to the
 * chip's ports in the half wave routine.
 *
 * Aggregate bit 1 values for each port depending on its value.
 *
 */

static void
prepare_device_data (void)
{
	int pixel, interval, port;
	
	/*
	 * Step 1: set port values to zero
	 */

	for (interval = 0; interval < (DIM_INTERVALS - 1); interval++) {
		for(port = 0;port < MCU_PORTS;port++) {
			dim_interval[interval].port_value[port] = 0;
		}
	}

	/*
	 * Step 2: set port value bit in the corresponding interval
	 */
	for (pixel = 0; pixel < MCU_PIXELS; pixel++) {
		if(Device->pixel[pixel][0]) {
			if (Device->pixel[pixel][0] > DIM_INTERVALS) {
				rt_printk("%s: Illegal pixel value (pixel=%d value=%d)\n", MCUDEV_NAME, pixel, Device->pixel[pixel][0]);
				continue;
			}
			interval = DIM_INTERVALS - Device->pixel[pixel][0];
			dim_interval[interval].port_value[pixel/8] |= (1 << (pixel % 8));
		}
	}

#ifdef OBSOLETE
	/*
	 * Step 3: accumulate port values (OBSOLETE as long as all intervals > pulse_length !!!)
	 */

	for (interval = 1; interval < (DIM_INTERVALS - 1); interval++) {
		for(port = 0;port < MCU_PORTS;port++) {
			dim_interval[interval].port_value[port] |= dim_interval[interval-1].port_value[port];
		}
	}
#endif
}


/*
 * Dimmer Process
 */

static void
mcudev_dim_process (int data)
{
	int wave, interval, port;
	int dim_phase_index = 0;
	RTIME current_phase;                              // start time of current phase

	setup_dim_interval();

	while(TRUE) {

		for(wave = 0;wave <= 1;wave++) {

			current_phase = phase_table[dim_phase_index];
			prepare_device_data();

//			rt_printk("time: %lld  phase: %lld\n", rt_get_time(), current_phase);

			for(interval = 0;interval < DIM_INTERVALS;interval++) {

				/*
				 * sleep until beginning of next interval
				 */
				rt_sleep_until(dim_interval[interval].time_offset + current_phase);

				/*
				 * write current interval's port values to ports
				 */
				for(port = 0;port < MCU_PORTS;port++) {
					outb(dim_interval[interval].port_value[port], pio_port_io_addr[port]);
				}
			}

			/*
			 * Go to next phase
			 */
			dim_phase_index = (dim_phase_index + 1) & (PHASE_TABLE_SIZE-1);

			if(phase_table[dim_phase_index] <= current_phase) {
				rt_printk("phase timer underrun (index=%d current=%lld next=%lld)\n",
					dim_phase_index, current_phase, phase_table[dim_phase_index]);
			}

		}
	}
}


/*
 * Interrupt Handler
 */

static void
phase_isr(void)
{
	static int dim_process_started = 0;
	static RTIME last_irq_activation_time = 0;
	static int irq_phase_index = 0;

	int status;

	RTIME irq_period;
	RTIME irq_activation_time;

	irq_activation_time = rt_get_time();

	if(last_irq_activation_time == 0) {
		last_irq_activation_time = irq_activation_time;
		rt_enable_irq(PHASE_IRQ);
		return;
	}

	irq_period = (irq_activation_time - last_irq_activation_time);
	phase_length = irq_period / 2;
	last_irq_activation_time = irq_activation_time;

	phase_table[irq_phase_index] = irq_activation_time + irq_delay + 4 * irq_period;
	phase_table[irq_phase_index+1] = phase_table[irq_phase_index] + phase_length;

	irq_phase_index = (irq_phase_index + 2) & (PHASE_TABLE_SIZE-1);

	if(!dim_process_started) {
		status = rt_task_resume(&mcudev_dim_task);
		if(status != 0) {
			rt_printk("?");
		} else {
			dim_process_started = 1;
			rt_printk("!");
		}
	}

	rt_enable_irq(PHASE_IRQ);
}


/*
 * release_memory
 * 
 * release all memory acquired by the module
 */

static void
release_memory(void)
{
	rtai_kfree(MCU_DEVICE_SHMEM_MAGIC);
}


/*
 * init_module
 * 
 * intialize driver module. acquire shared memory area to get in contact with mcud
 * enable phase syncronization IRQ and set up IRQ service routine
 * set up one shot real time process for actual port handling
 */

int
init_module(void)
{
	int status, pixel;
	unsigned long startup;


	printk ("%s: init_module\n", MCUDEV_NAME);

	Device = (mcu_device_t *) rtai_kmalloc(MCU_DEVICE_SHMEM_MAGIC, sizeof(mcu_device_t));
	if (Device == NULL) {
		printk ("%s: rtai_kmalloc failed\n", MCUDEV_NAME);
		return 1;
	}
	
	strcpy(Device->name, MCUDEV_NAME);
	Device->depth = 1;
	Device->maxval = DIM_RESOLUTION - 1;
	Device->flag_zero_off = FALSE;

	mcu_setup_pio();

	/*
	 * convert parameter ns values to internal count units
	 */

	irq_delay = nano2count(irq_delay_ns);
	pulse_length = nano2count(pulse_length_ns);

	printk("%s: irq_delay=%d ns pulse_length=%d ns init_value=%d\n", MCUDEV_NAME, irq_delay_ns, pulse_length_ns, init_value);

	for(pixel = 0;pixel < MCU_PIXELS; pixel++) {
		Device->pixel[pixel][0] = init_value;
	}

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
		printk("%s: rt_request_global_irq failed\n", MCUDEV_NAME);
		release_memory();
		return 1;
	}
	startup = rt_startup_irq(PHASE_IRQ);
	printk("%s: rt_startup_irq returned 0x%04lx\n", MCUDEV_NAME, startup);
	rt_enable_irq(PHASE_IRQ);
	rt_global_sti();

	/*
	 * Initialize real time process
	 */
	status = rt_task_init(&mcudev_dim_task, &mcudev_dim_process, 0, 4096, 1, 0, NULL);
	if(status != 0) {
		printk("%s: rt_task_init failed (%d)\n", MCUDEV_NAME, status);
		release_memory();
		return 1;
	}

	rt_set_oneshot_mode();
	start_rt_timer(0);

	printk ("%s: dimmer task started\n", MCUDEV_NAME);

	Device->flag_loaded = TRUE;
	return 0;
}


/*
 * cleanup_module
 *
 * shut down real-time process
 * shut down interrupt service routine
 * release memory and terminate module
 */

void
cleanup_module (void)
{
	int status;
	
	Device->flag_loaded = FALSE;

	release_memory();

	/*
	 * Disable IRQ
	 */
	rt_disable_irq(PHASE_IRQ);
	status = rt_free_global_irq(PHASE_IRQ);
	if (status != 0) {
		printk("%s: rt_free_global_irq failed (%d)\n", MCUDEV_NAME, status);
	}

	/*
	 * Terminate Real Time Process
	 */
	status = rt_task_delete(&mcudev_dim_task);
	if (status != 0) {
		printk("%s: rt_task_delete failed (%d)\n", MCUDEV_NAME, status);
	}

	/*
	 * Switch off all lamps
	 */
	mcu_set_io_ports(0x00);

	printk ("%s: cleanup_module (phase_length: %lld ns)\n", MCUDEV_NAME, count2nano(phase_length) );

	return;
}

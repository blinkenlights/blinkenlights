/*
 * mcudev_pdim
 *
 * MCU Driver Module for dimmed lightbulbs using periodic timing (steps).
 *
 * The interrupt service routing reacts to a 50Hz interrupt driven
 * by the phase change of the alternating current driving the lights.
 *
 * The ISR re-adjusts the phase_length continuously and that way
 * provides the necessary parameters for the light-controlling
 * real-time process to synchronize with the phase.
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

#define MCUDEV_NAME            "mcu_pdim"
#define MCU_PIXELS              48                 // number of pixels supported
#define PHASE_IRQ               15                 // IRQ to be triggered by phase change


/*
 * module parameters
 */

#define IRQ_DELAY_NS            580000            // time to wait until next phase starts after IRQ occurred
int irq_delay_ns = IRQ_DELAY_NS;
MODULE_PARM (irq_delay_ns, "1i");

#define IRQ_DELAY_STEP          6                 // steps to wait until next phase starts after IRQ occurred
int irq_delay_step = IRQ_DELAY_STEP;
MODULE_PARM (irq_delay_step, "1i");

#define DIM_STEP_LENGTH_NS      100740            // length of a single step in ns (10ms / DIM_STEPS)
int dim_step_length_ns = DIM_STEP_LENGTH_NS;
MODULE_PARM (dim_step_length_ns, "1i");

#define PULSE_LENGTH_STEPS      1                 // pulse length in number of steps
int pulse_length_steps = PULSE_LENGTH_STEPS;
MODULE_PARM (pulse_length_steps, "1i");

int init_value = 0;
MODULE_PARM (init_value, "1i");

// char * phases = "000000000000000000000000000000000000000000000000";
char * phases = "000000000000000000000222222222222222222222222222";
MODULE_PARM (phases, "s");

/*
 * globals
 */

static RT_TASK mcudev_pdim_task;
static mcu_device_t * Device;

static RTIME irq_delay;                                   // IRQ delay time
static RTIME phase_length;                                // current phase length
static RTIME dim_step_length;                             // dim step length

static int ignition = FALSE;

/*
 * Dimmer tables
 */

#define DIM_RESOLUTION          16
#define DIM_INTERVAL_MAP        dim_interval_map_16

#define DIM_LAST_INTERVAL       (DIM_RESOLUTION - 1)      // last interval: forces all outputs to zero!

#define DIM_STEPS               99        // total number of steps per phase

#define DIM_PHASE_STEP_OFFSET   33        // offset in steps between phases


/*
 * dim tables
 *
 * contain a step index for each interval indicating when the interval
 * should begin within the step raster.
 *
 * One step is 100us.
 */

static int dim_interval_map_16[16] = {
//	  0, 17,  7, 6, 5, 5, 4, 4, 4, 4, 4, 5, 5, 6, 7, 17
	  0, 30,  8, 7, 6, 5, 5, 4, 4, 4, 3, 4, 4, 5, 6, 5
};

/*
 * optional tables
 *
static int dim_interval_map_32[32] = {
	  0, 12,  5,  4,  3,  3,  3,  3,  2,  2,  2,  2,  2,  2,  2,  2,
	  2,  2,  2,  2,  2,  2,  2,  2,  2,  3,  3,  3,  3,  4,  5, 12
};

static int dim_interval_map_64[64] = {
      0,  8,  4,  3,  2,  2,  2,  2,  2,  2,  1,  1,  1,  1,  1,  1,
	  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,  1,
	  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  3,  4,  8
};
 */

/*
 * dim step table
 *
 * the dim step table gets initialized once based on the phase assignment
 * done at module startup. each step contains the information to which
 * interval of each of the phases the step refers.
 *
 * the interval value of each phase is used to gather the current output
 * values for each lamp. the information for each phase is or'ed together
 * before being written to the ports.
 */

#define DIM_STEP_NOOP           -1
 
typedef struct dim_step {
	int interval[3];                       // index to corresponding interval per phase
	int no_change;                         // flag indicating any intervals are indexed. maybe useless.
} dim_step_t;

static dim_step_t dim_step[DIM_STEPS];


/*
 * dim_interval
 *
 * the dim interval table provides per-interval data. intervals relate
 * to brightness values in reverse order: the first interval is the one
 * setting maximum brightness because its switches on the light at
 * phase start. the last interval is the one representing the off state
 * also resetting all output values because the semiconductor relay memorizes
 * the output bit until the next phase starts.
 *
 * There is one set of port values for each phase and each interval. the
 * data gets calculated at the end of every phase.
 */

typedef struct dim_interval {
	unsigned char port_value[MCU_PORTS];         // data to be set in this interval
} dim_interval_t;

/*
 * dim_phase
 */

typedef enum phase_index {
	PHASE_R = 0,
	PHASE_S = 1,
	PHASE_T = 2
} phase_index_t;

typedef struct dim_phase {
	dim_interval_t interval[DIM_RESOLUTION];  // table of interval related data
    unsigned char port_mask[MCU_PORTS];       // mask indicating which port belongs to which phase
} dim_phase_t;

static dim_phase_t dim_phase[3];                  // per-phase storage

/*
 * pixel phase configuration
 */

static phase_index_t pixel_phase[MCU_PIXELS];                   // maps pixels to phases


/*
 * setup dim step table
 *
 * initialize the constant values of the table
 */

static void
setup_dim_step_table()
{
	int step, step_offset, interval, phase;

	/*
	 * initialize all steps to cause "no change"
	 */

	for (step = 0; step < DIM_STEPS; step++) {
		for (phase = 0;phase < 3;phase++) {
			dim_step[step].interval[phase] = DIM_STEP_NOOP;
		}
		dim_step[step].no_change = TRUE;
	}

	/*
	 * mark steps that activate new intervals
	 */

	for (phase = 0, step_offset = 0;phase < 3;phase++, step_offset += DIM_PHASE_STEP_OFFSET) {
		for (interval = 0, step = step_offset;interval < DIM_RESOLUTION;interval++) {
			step += (interval == DIM_LAST_INTERVAL ? pulse_length_steps : DIM_INTERVAL_MAP[interval]);
			dim_step[step % DIM_STEPS].interval[phase] = interval;
			dim_step[step % DIM_STEPS].no_change = FALSE;
		}
	}
}



/*
 * prepare_port_values
 *
 * read in current pixel values and prepare chip-oriented data aggregation
 * producing an array of values that can be written directly to the
 * chip's ports in the dimmer process.
 *
 */

static void
prepare_port_values (int phase)
{
	int pixel, interval, port;
	dim_phase_t * p_phase = &dim_phase[phase];

	/*
	 * set port values for each interval to zero
	 */

	for (interval = 0; interval < DIM_RESOLUTION; interval++) {
		for (port = 0;port < MCU_PORTS;port++) {
			p_phase->interval[interval].port_value[port] = 0;
		}
	}

	/*
	 * set port value bit in the corresponding interval port value
	 */
	for (pixel = 0; pixel < MCU_PIXELS; pixel++) {
		if (pixel_phase[pixel] != phase)
			continue;

		if (Device->pixel[pixel][0]) {
			if (Device->pixel[pixel][0] > DIM_LAST_INTERVAL) {
				rt_printk("%s: Illegal pixel value (pixel=%d value=%d)\n", MCUDEV_NAME, pixel, Device->pixel[pixel][0]);
				continue;
			}

			interval = DIM_LAST_INTERVAL - Device->pixel[pixel][0];
			p_phase->interval[interval].port_value[pixel/8] |= (1 << (pixel % 8));
		}
	}

}


/*
 * Dimmer Process
 */

static void
mcudev_pdim_process (int data)
{
	int interval, phase, port;
	unsigned char value;
	int current_step = 0;

	unsigned char port_value[MCU_PORTS];                 // buffer for new data to be written to the ports
	unsigned char port_mask[MCU_PORTS];                  // buffer for port mask

	unsigned char last_port_value[MCU_PORTS];            // buffer for last values being written to the ports

	rt_printk ("%s: mcudev_pdim task started\n", MCUDEV_NAME);

	/*
	 * initialize data structures
	 */
	prepare_port_values(0);
	prepare_port_values(1);
	prepare_port_values(2);

	memset(&last_port_value, 0, MCU_PORTS * sizeof(unsigned char) );

	while (TRUE) {
		if (ignition) {
			ignition = FALSE;
#ifdef REPORT_STEP_ADJUSTMENT
			if(current_step != irq_delay_step) {
				printk ("%s: step adjustment: %d -> %d\n", MCUDEV_NAME, current_step, irq_delay_step);
			}
#endif
			current_step = irq_delay_step;
		}

		if (dim_step[current_step].no_change == FALSE) {
	
			/*
			 * determine current port values and port mask
			 */
		
			memset(&port_value, 0, MCU_PORTS * sizeof(unsigned char) );
			memset(&port_mask,  0, MCU_PORTS * sizeof(unsigned char) );
		
			for (phase = 0;phase < 3;phase++) {	
				interval = dim_step[current_step].interval[phase];
	
				if (interval != DIM_STEP_NOOP) {
					for (port = 0;port < MCU_PORTS;port ++) {
						port_value[port] |= dim_phase[phase].interval[interval].port_value[port];
						if (interval == DIM_LAST_INTERVAL) {
							/*
							 * force phase's bit to 0 in lsat interval
							 */
							port_mask[port] |= dim_phase[phase].port_mask[port];
						}
					}
				}
			}
	
	
			/*
			 * write current step's port values to ports if they have changed
			 */
			for (port = 0;port < MCU_PORTS;port++) {
				value = (last_port_value[port] & ~port_mask[port]) | port_value[port];
				if (value != last_port_value[port]) {
					outb(value, pio_port_io_addr[port]);
					last_port_value[port] = value;
				}
			}

		}

		/*
		 * move to next step.
		 * check if one of the phases switches to interval 0 and if so, prepare
		 * the port values of the phase according to the current pixel data.
		 */

		current_step = (current_step + 1) % DIM_STEPS;

		for (phase = 0;phase < 3;phase++) {
			if (dim_step[current_step].interval[phase] == 0) {
				prepare_port_values(phase);
			}
		}

		/*
		 * wait for next period
		 */
		
		rt_task_wait_period();

	}
}


/*
 * Interrupt Handler
 */

static void
phase_isr(void)
{
#ifdef STATISTICS
	static RTIME last_irq_activation_time = 0;
	int status;
	RTIME irq_period;
	RTIME irq_activation_time;
#endif

	ignition = TRUE;

#ifdef STATISTICS
	irq_activation_time = rt_get_time();

	if (last_irq_activation_time == 0) {
		last_irq_activation_time = irq_activation_time;
		rt_enable_irq(PHASE_IRQ);
		return;
	}

	irq_period = (irq_activation_time - last_irq_activation_time);
	phase_length = irq_period / 2;
	last_irq_activation_time = irq_activation_time;
#endif

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
 * set up periodic real time process for actual port handling
 */

int
init_module(void)
{
	int status, pixel, port, phase, bit, step;
	unsigned long startup;
	RTIME period, current_time;

	printk ("%s: init_module\n", MCUDEV_NAME);

	Device = (mcu_device_t *) rtai_kmalloc(MCU_DEVICE_SHMEM_MAGIC, sizeof(mcu_device_t));
	if (Device == NULL) {
		printk ("%s: rtai_kmalloc failed\n", MCUDEV_NAME);
		return 1;
	}
	
	strcpy(Device->name, "mcudev_pdim");
	Device->depth = 1;
	Device->maxval = DIM_RESOLUTION - 1;
	Device->flag_zero_off = FALSE;

	/*
	 * setup pixel phase configuration
	 */

	if (strlen(phases) != MCU_PIXELS) {
		printk ("%s: illegal number of pixels configured (%d). Should be %d.\n", MCUDEV_NAME, strlen(phases), MCU_PIXELS);
		release_memory();
		return 1;
	}

	for (port = 0, pixel = 0;pixel < MCU_PIXELS;pixel++) {
		if ( phases[pixel] != '0' && phases[pixel] != '1' && phases[pixel] != '2') {
			printk ("%s: illegal phase number (%c).\n", MCUDEV_NAME, phases[pixel]);
			release_memory();
			return 1;
		}

		pixel_phase[pixel] = phases[pixel] - '0';
		dim_phase[pixel_phase[pixel]].port_mask[pixel / 8] |= ( 1 << (pixel % 8));
	}

	/*
	 * Print resulting mask for verification
	 */
	for (phase = 0;phase < 3; phase++) {
		printk ("%s: phase %d pixel mask: ", MCUDEV_NAME, phase);
		for (port = 0;port < MCU_PORTS;port++) {
			for ( bit = 0;bit < 8;bit++) {
				if ((dim_phase[phase].port_mask[port] & (1 << bit)) )
					printk ("1");
				else
					printk ("0");
			}
			printk (" [%02x] ", dim_phase[phase].port_mask[port]);
		}
		printk ("\n");
	}

	/*
	 * set up dim step table
	 */
	setup_dim_step_table();

	for (step = 0; step < DIM_STEPS;step++) {
		printk ("%s: step table: step %3d: ", MCUDEV_NAME, step);
		for (phase = 0;phase < 3; phase++) {
			printk ("%2d ", dim_step[step].interval[phase]);
		}
		printk ("\n");
	}
	/*
	 * initialize MCU PIO subsystem
	 */

	mcu_setup_pio();

	/*
	 * convert parameter ns values to internal count units
	 */

	irq_delay = nano2count(irq_delay_ns);
	dim_step_length = nano2count(dim_step_length_ns);

	printk("%s: irq_delay_step=%d  dim_step_length=%d ns  pulse_length_steps=%d ns  init_value=%d\n",
		    MCUDEV_NAME, irq_delay_step, dim_step_length_ns, pulse_length_steps, init_value);

	for (pixel = 0;pixel < MCU_PIXELS; pixel++) {
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
	if (status != 0) {
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
	status = rt_task_init(&mcudev_pdim_task, &mcudev_pdim_process, 0, 4096, 1, 0, NULL);
	if (status != 0) {
		printk("%s: rt_task_init failed (%d)\n", MCUDEV_NAME, status);
		release_memory();
		return 1;
	}

	rt_set_periodic_mode();

	period = start_rt_timer(dim_step_length);
	current_time = rt_get_time();

	status = rt_task_make_periodic(&mcudev_pdim_task, current_time + period, dim_step_length);
	if (status != 0) {
		rt_printk("%s: rt_task_make_periodic failed (%d)\n", MCUDEV_NAME, status);
		release_memory();
		return 1;
	}

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
	status = rt_task_delete(&mcudev_pdim_task);
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

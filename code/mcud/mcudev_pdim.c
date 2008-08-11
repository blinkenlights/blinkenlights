/*
 * mcudev_pdim
 *
 * MCU Driver Module for dimmed lightbulbs using RTAI periodic timing (steps).
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

MODULE_AUTHOR("Tim Pritlove");
MODULE_DESCRIPTION("MCU Driver Module for dimmed lightbulbs using RTAI periodic timing");
MODULE_LICENSE("GPL");

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>
#include <rtai_shm.h>

#include "mcu.h"
#include "pio.h"

#include "mcud.h"
#include "mcudev.h"

#define MCUDEV_NAME            "mcu_pdim"
#define PHASE_IRQ              15                 // IRQ to be triggered by phase change


/*
 * MODULE PARAMETERS
 */

/*
 * irq_delay_step
 *
 * steps to wait until next phase starts after IRQ occurred
 */
#define IRQ_DELAY_STEP          0
int irq_delay_step = IRQ_DELAY_STEP;
MODULE_PARM (irq_delay_step, "1i");

/*
 * dim_step_length_ns
 *
 * length of a single step in ns (10ms / DIM_STEPS)
 */
#define DIM_STEP_LENGTH_NS      101010
int dim_step_length_ns = DIM_STEP_LENGTH_NS;
MODULE_PARM (dim_step_length_ns, "1i");

/*
 * pulse_length_steps
 *
 * pulse length in number of steps
 */
#define PULSE_LENGTH_STEPS      2
int pulse_length_steps = PULSE_LENGTH_STEPS;
MODULE_PARM (pulse_length_steps, "1i");

/*
 * dim_resolution
 *
 * resolution of dimmer function. number of intervals.
 */
#define MAX_DIM_RESOLUTION   8

int dim_resolution = 8;
MODULE_PARM (dim_resolution, "1i");

/*
 * dim_steps
 *
 * interval map for dimmer function
 */

#define DIM_STEPS_DEFAULT   { 0, 40, 10, 7, 5, 5, 6, 26 }

int dim_steps0[MAX_DIM_RESOLUTION] = DIM_STEPS_DEFAULT;
MODULE_PARM (dim_steps0, "1-" __MODULE_STRING(MAX_DIM_RESOLUTION) "1i");

int dim_steps1[MAX_DIM_RESOLUTION] = DIM_STEPS_DEFAULT;
MODULE_PARM (dim_steps1, "1-" __MODULE_STRING(MAX_DIM_RESOLUTION) "1i");

int dim_steps2[MAX_DIM_RESOLUTION] = DIM_STEPS_DEFAULT;
MODULE_PARM (dim_steps2, "1-" __MODULE_STRING(MAX_DIM_RESOLUTION) "1i");

/*
 * init_mcu
 *
 * boolean determining if MCU PIO subsystem should be initialized on AMD ELan Controller
 */
int init_mcu = 1;
MODULE_PARM (init_mcu, "1i");

/*
 * init_value
 *
 * global initial value for all lamps
 */
int init_value = -1;
MODULE_PARM (init_value, "1i");

/*
 * init_values
 *
 * individual initial value for all lamps
 */
int init_values[MCU_MAX_LAMPS];
MODULE_PARM (init_values, "1-" __MODULE_STRING(MCU_MAX_LAMPS) "i");

/*
 * max_lamps
 *
 * maximum number of lamps supported
 */
int max_lamps = MCU_MAX_LAMPS;
MODULE_PARM (max_lamps, "1i");

/*
 * mcu_phase
 *
 * phase setting (0, 1, 2) for the MCU
 */
int mcu_phase = 0;
MODULE_PARM (mcu_phase, "1i");

/*
 * phase1_offset
 * phase2_offset
 *
 * phase setting (0, 1, 2) for the MCU
 */
int phase1_offset = 33;
MODULE_PARM (phase1_offset, "1i");
int phase2_offset = 66;
MODULE_PARM (phase2_offset, "1i");

/*
 * spark_mode
 *
 * set signal to zero after interval start
 */
int spark_mode = 0;
MODULE_PARM (spark_mode, "1i");

/*
 * phases
 *
 * phase setting (0, 1, 2) for each lamp
 */
char * phases = "000000000000000000000000000000000000000000000000";
MODULE_PARM (phases, "s");

/*
 * globals
 */

static RT_TASK mcudev_pdim_task;
static mcu_device_t * Device;

#ifdef STATISTICS
static RTIME last_phase_length;
static RTIME min_phase_length = 0;
static RTIME max_phase_length = 0;
static RTIME average_phase_length = 0;
#endif

static RTIME dim_step_length;                             // dim step length
static int dim_last_interval;

static int phase_offset[3] = { 0, 33, 66 };

static int ignition = FALSE;

/*
 * Dimmer tables
 */

#define DIM_STEPS               99        // total number of steps per phase


/*
 * dim tables
 *
 * contain a step index for each interval indicating when the interval
 * should begin within the step raster.
 *
 * One step is 100us.
 */

static int dim_interval_map[3][MAX_DIM_RESOLUTION];

/*
static int dim_interval_map_8[3][8] = {
	{ 0, 40, 10, 7, 5, 5, 6, 26 },
	{ 0, 40, 10, 7, 5, 5, 6, 26 },
	{ 0, 40, 10, 7, 5, 5, 6, 26 }
};

static int dim_interval_map_16[3][16] = {
// linear (calculated) intervals
//	  0, 17,  7, 6, 5, 5, 4, 4, 4, 4, 4, 5, 5, 6, 7, 17

// manually adjusted intervals
//	  0, 30,  8, 7, 6, 5, 5, 4, 4, 4, 3, 4, 4, 5, 6, 5

// ARCADE Setup
	 { 0, 32, 8, 7, 5, 5, 4, 4, 4, 3, 3, 3, 4, 5, 5, 7 },
	 { 0, 32, 8, 7, 5, 5, 4, 4, 4, 3, 3, 3, 4, 5, 5, 7 },
	 { 0, 32, 8, 7, 5, 5, 4, 4, 4, 3, 3, 3, 4, 5, 5, 7 }
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
	int interval[3];            // index to corresponding interval per phase
	int no_change;              // flag indicating any intervals are indexed
	unsigned char port_value[PIO_MAX_PORTS]; // DEBUG: port value log
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
	unsigned char port_value[PIO_MAX_PORTS];         // data to be set in this interval
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
	dim_interval_t interval[MAX_DIM_RESOLUTION];  // table of interval related data
    unsigned char port_mask[PIO_MAX_PORTS];       // mask indicating which port belongs to which phase
} dim_phase_t;

static dim_phase_t dim_phase[3];              // per-phase storage

/*
 * lamp phase configuration
 */

static phase_index_t lamp_phase[MCU_MAX_LAMPS];                   // maps lamps to phases


/*
 * setup dim step table
 *
 * Initialize the constant values of the dim step table.
 * Usually called at startup and for calibration at runtime.
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

	for (phase = 0;phase < 3;phase++) {
		step_offset = phase_offset[phase];
		for (interval = 0, step = step_offset;interval < dim_resolution;interval++) {
			step += (interval == dim_last_interval ? pulse_length_steps : dim_interval_map[phase][interval]);
			dim_step[step % DIM_STEPS].interval[phase] = interval;
			dim_step[step % DIM_STEPS].no_change = FALSE;
		}
	}

	/*
	 * print new table content
	 */

	for (step = 0; step < DIM_STEPS;step++) {
		printk ("%s: step table: step %3d: ", MCUDEV_NAME, step);
		for (phase = 0;phase < 3; phase++) {
			printk ("%2d ", dim_step[step].interval[phase]);
		}
		printk ("\n");
	}

	return;
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
	int lamp, interval, port;
	dim_phase_t * p_phase = &dim_phase[phase];

	/*
	 * set port values for each interval to zero
	 */

	for (interval = 0; interval < dim_resolution; interval++) {
		for (port = 0;port < PIO_MAX_PORTS;port++) {
			p_phase->interval[interval].port_value[port] = 0;
		}
	}

	/*
	 * set port value bit in the corresponding interval port value
	 */
	for (lamp = 0; lamp < max_lamps; lamp++) {
		if (lamp_phase[lamp] != phase)
			continue;

		if (Device->pixel[lamp][0]) {
			if (Device->pixel[lamp][0] > dim_last_interval) {
				rt_printk("%s: Illegal lamp value (lamp=%d value=%d)\n", MCUDEV_NAME, lamp, Device->pixel[lamp][0]);
				continue;
			}

			interval = dim_last_interval - Device->pixel[lamp][0];
			p_phase->interval[interval].port_value[lamp/8] |= (1 << (lamp % 8));
		}
	}

}

/*
 * DEBUG: dump_port_values
 */

static void
dump_port_values(void)
{
	int phase, interval, port, bit;

	printk ("Dumping per-phase interval port values:\n");
	for (phase = 0;phase < 3;phase++) {
		for (interval = 0; interval < dim_resolution; interval++) {
			printk ("phase %1d interval %2d: ", phase, interval);
			for (port = 0;port < PIO_MAX_PORTS;port++) {
				for ( bit = 0;bit < 8;bit++) {
					if ((dim_phase[phase].interval[interval].port_value[port] & (1 << bit)) )
						printk ("1");
					else
						printk ("0");
				}
				printk (" ");
			}
			printk ("\n");
		}
	}
}


/*
 * Dimmer Process
 */

static void
mcudev_pdim_process (int data)
{
//	int first_sync = FALSE;
	int interval, phase, port;
	unsigned char value;
	int current_step = 0;

	unsigned char port_value[PIO_MAX_PORTS];                 // buffer for new data to be written to the ports
	unsigned char port_mask[PIO_MAX_PORTS];                  // buffer for port mask

	unsigned char last_port_value[PIO_MAX_PORTS];            // buffer for last values being written to the ports

	rt_printk ("%s: real time task started\n", MCUDEV_NAME);

	/*
	 * initialize data structures for every phase
	 */
	prepare_port_values(0);
	prepare_port_values(1);
	prepare_port_values(2);

	memset(&last_port_value, 0, PIO_MAX_PORTS * sizeof(unsigned char) );

	while (TRUE) {
		if (ignition) {
			ignition = FALSE;
			if (current_step != irq_delay_step) {
//				if ( first_sync == FALSE || (abs(irq_delay_step - current_step) < 10) ) {
#ifdef REPORT_STEP_ADJUSTMENT
					printk ("%s: step adjustment: %d -> %d\n", MCUDEV_NAME, current_step, irq_delay_step);
#endif
					current_step = irq_delay_step;
//					first_sync = TRUE;
//				}
			}
		}

		if (dim_step[current_step].no_change == FALSE) {
	
			/*
			 * determine current port values and port mask
			 */
		
			memset(&port_value, 0, PIO_MAX_PORTS * sizeof(unsigned char) );
			memset(&port_mask,  0, PIO_MAX_PORTS * sizeof(unsigned char) );
		
			for (phase = 0;phase < 3;phase++) {	
				interval = dim_step[current_step].interval[phase];
	
				for (port = 0;port < PIO_MAX_PORTS;port ++) {
					if (interval == DIM_STEP_NOOP) {
						/*
						 * force port bits to zero
						 */
						if (spark_mode) {
							port_mask[port] |= dim_phase[phase].port_mask[port];
						}
					} else {
						port_value[port] |= dim_phase[phase].interval[interval].port_value[port];
						if (interval == dim_last_interval) {
							/*
							 * force phase's bit to 0 in last interval
							 */
							port_mask[port] |= dim_phase[phase].port_mask[port];
						}
					}
				}
			}
	
			/*
			 * write current step's port values to ports if they have changed
			 */
			for (port = 0;port < PIO_MAX_PORTS;port++) {
				value = (last_port_value[port] | port_value[port]) & ~port_mask[port];
				if (value != last_port_value[port]) {
					outb(value, pio_port_addr[port]);
					last_port_value[port] = value;
				}
			}
		}
		memcpy (&dim_step[current_step].port_value, &last_port_value, sizeof(last_port_value));

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
	last_phase_length = irq_period / 2;
	if (min_phase_length == 0 || last_phase_length < min_phase_length) {
		min_phase_length = last_phase_length;
	}
	if (last_phase_length > max_phase_length) {
		max_phase_length = last_phase_length;
	}
	last_irq_activation_time = irq_activation_time;

	if (average_phase_length == 0)
		average_phase_length = last_phase_length;

	average_phase_length = (average_phase_length + last_phase_length) / 2;

	if (last_phase_length < nano2count(8000000) )
		ignition = FALSE;
#endif
#ifdef REPORT_IRQ
	rt_printk("*");
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
 * Initialize driver module. Acquire shared memory area to get in contact with mcud.
 * Enable phase syncronization IRQ and set up IRQ service routine.
 * Set up periodic real time process for actual port handling.
 */

int
init_module(void)
{
	int status, lamp, port, phase, bit, interval;
	unsigned long startup;
	RTIME period, current_time;

	printk ("%s: init_module\n", MCUDEV_NAME);


	/* HACKHACKHACK: switch from pull-up to pull-down!!! */
	printk ("setting up poll-down resistor\n");

	cli ();
	outw (0x00ca, 0x22);
	outw (0x00ca, 0x22);
	outw (0x00ca, 0x22);

	outw (0x01e5, 0x22);
	sti ();

	Device = (mcu_device_t *) rtai_kmalloc(MCU_DEVICE_SHMEM_MAGIC, sizeof(mcu_device_t));
	if (Device == NULL) {
		printk ("%s: rtai_kmalloc failed\n", MCUDEV_NAME);
		return 1;
	}
	
	if ( mcu_phase < 0 && mcu_phase > 2) {
		printk ("%s: illegal mcu_phase number (%d).\n", MCUDEV_NAME, mcu_phase);
		release_memory();
		return 1;
	}


	/*
	 * setup lamp phase configuration
	 */

	if (strlen(phases) != max_lamps) {
		printk ("%s: illegal number of lamps configured (%d). Should be %d.\n", MCUDEV_NAME, strlen(phases), max_lamps);
		release_memory();
		return 1;
	}

	for (port = 0, lamp = 0;lamp < max_lamps;lamp++) {
		if ( phases[lamp] != '0' && phases[lamp] != '1' && phases[lamp] != '2') {
			printk ("%s: illegal phase number (%c).\n", MCUDEV_NAME, phases[lamp]);
			release_memory();
			return 1;
		}

		/*
		 * set lamp phase relative to MCU phase
		 */
		lamp_phase[lamp] = ((phases[lamp] - '0') + 3 - mcu_phase) % 3;
		dim_phase[lamp_phase[lamp]].port_mask[lamp / 8] |= ( 1 << (lamp % 8));
	}

	/*
	 * Print resulting mask for verification
	 */
	for (phase = 0;phase < 3; phase++) {
		printk ("%s: phase %d lamp mask: ", MCUDEV_NAME, phase);
		for (port = 0;port < PIO_MAX_PORTS;port++) {
			for ( bit = 0;bit < 8;bit++) {
				if ((dim_phase[phase].port_mask[port] & (1 << bit)) )
					printk ("1");
				else
					printk ("0");
			}
			printk (" ");
		}
		printk ("\n");
	}

	/*
	 * convert parameter ns values to internal count units
	 */

	dim_step_length = nano2count(dim_step_length_ns);

	/*
	 * setup dim_interval map based on parameters
	 */

	if (dim_resolution > MAX_DIM_RESOLUTION) {
		printk ("%s: dim_resolution exceeds maximum (%d).\n", MCUDEV_NAME, dim_resolution);
		release_memory();
		return 1;
	}
	dim_last_interval = dim_resolution -1;

	memcpy (&dim_interval_map[0], &dim_steps0, dim_resolution * sizeof(int));
	memcpy (&dim_interval_map[1], &dim_steps1, dim_resolution * sizeof(int));
	memcpy (&dim_interval_map[2], &dim_steps2, dim_resolution * sizeof(int));

	/*
	 * setup Device structure
	 */

	strcpy(Device->name, MCUDEV_NAME);
	Device->channels = 1;
	Device->maxval = dim_last_interval;
	Device->flag_zero_off = FALSE;
	Device->flag_loaded = TRUE;

	/*
	 * setup init_values
	 */

	for (lamp = 0;lamp < max_lamps; lamp++) {
		/*
		 * prevent illegal values for init_values
		 */
		if (init_values[lamp] > Device->maxval) {
			init_values[lamp] = Device->maxval;
		}
		Device->pixel[lamp][0] = init_values[lamp];
		printk("%s: init_value[%d]=%d\n", MCUDEV_NAME, lamp, init_values[lamp]);
	}

	/*
	 * set global init_value for all lamps
	 * prevent illegal values for init_value
	 */
	if (init_value != -1) {
		if (init_value > Device->maxval) {
			init_value = Device->maxval;
		}
		for (lamp = 0;lamp < max_lamps; lamp++) {
			Device->pixel[lamp][0] = init_value;
		}
	}

	/*
	 * set up phase offset table
	 */
	
	phase_offset[1] = phase1_offset;
	phase_offset[2] = phase2_offset;

	/*
	 * set up dim step table
	 */
	setup_dim_step_table();

	{
		/*
		 * DEBUG: print calculated port values based on init_values
		 */
		prepare_port_values(0);
		prepare_port_values(1);
		prepare_port_values(2);

		dump_port_values();
	}

	/*
	 * initialize MCU PIO and IRQ subsystem
	 */
	if (init_mcu) {
		mcu_setup();

		/*
		 * Set DIL/NetPC INT1 (SC410 PIRQ3) to IRQ PHASE_IRQ
		 */
		csc_write(ICR_B, (csc_read (ICR_B) & PIRQ2S) | (PHASE_IRQ << 4));
	}
	pio_setup();

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
		printk("%s: rt_task_make_periodic failed (%d)\n", MCUDEV_NAME, status);
		release_memory();
		return 1;
	}


	/*
	 * report parameter setup
	 */
	printk("%s: dim_resolution=%d\n", MCUDEV_NAME, dim_resolution);
	for (phase = 0;phase < 3;phase++) {
		printk("%s: dim_steps%d=", MCUDEV_NAME, phase);
		for (interval = 0;interval < dim_resolution;interval++) {
			printk("[%2d] ", dim_interval_map[phase][interval]);
		}
		printk("\n");
	}
	printk("%s: irq_delay_step=%d\n", MCUDEV_NAME, irq_delay_step);
	printk("%s: init_value=%d\n", MCUDEV_NAME, init_value);
	printk("%s: mcu_phase=%d\n", MCUDEV_NAME, mcu_phase);
	printk("%s: phase_offset[]=[%2d][%2d][%2d]\n", MCUDEV_NAME, phase_offset[0], phase_offset[1], phase_offset[2]);
	printk("%s: max_lamps=%d\n", MCUDEV_NAME, max_lamps);
	printk("%s: dim_step_length_ns=%d\n",	MCUDEV_NAME, dim_step_length_ns);
	printk("%s: dim_step_length=%lld\n",	MCUDEV_NAME, dim_step_length);
	printk("%s: pulse_length_steps=%d\n", MCUDEV_NAME, pulse_length_steps);

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

	{
		/*
		 * DEBUG: print logged port values
		 */

		int step, port, bit;

		printk ("Dumping last port values:\n");
		for (step = 0;step < DIM_STEPS;step++) {
			printk ("step %2d: ", step);
			for (port = 0;port < PIO_MAX_PORTS;port++) {
				for ( bit = 0;bit < 8;bit++) {
					if ((dim_step[step].port_value[port] & (1 << bit)) )
						printk ("1");
					else
						printk ("0");
				}
				printk (" ");
			}
			printk ("\n");
		}
	}

	/*
	 * Shut down driver
	 */
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
	pio_init_ports(0x00);

	printk ("%s: cleanup_module\n", MCUDEV_NAME );

#ifdef STATISTICS
	printk ("%s: last_phase_length: %lld ns\n", MCUDEV_NAME, count2nano(last_phase_length) );
	printk ("%s: min_phase_length:  %lld ns\n", MCUDEV_NAME, count2nano(min_phase_length) );
	printk ("%s: max_phase_length:  %lld ns\n", MCUDEV_NAME, count2nano(max_phase_length) );
	printk ("%s: average_phase_length:  %lld ns\n", MCUDEV_NAME, count2nano(average_phase_length) );
#endif

	return;
}

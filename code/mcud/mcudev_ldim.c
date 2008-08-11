/*
 * mcudev_ldim
 *
 * MCU Driver Module for dimmed LEDs using RTAI periodic timing (steps).
 *
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

#define MCUDEV_NAME            "mcu_ldim"

/*
 * MODULE PARAMETERS
 */

/*
 * dim_step_length_ns
 *
 * length of a single step in ns (1s / 50 = 40ms / DIM_STEPS = 78,125ns)
 */
#define DIM_STEP_LENGTH_NS      40000
int dim_step_length_ns = DIM_STEP_LENGTH_NS;
MODULE_PARM (dim_step_length_ns, "1i");

/*
 * dim_resolution
 *
 * resolution of dimmer function. number of intervals.
 */
#define MAX_DIM_RESOLUTION   256

int dim_resolution = 16;
MODULE_PARM (dim_resolution, "1i");

/*
 * dim_steps
 *
 * interval map for dimmer function
 */

#define DIM_STEPS_DEFAULT_8   { 0, 32, 32, 32, 32, 32, 32, 32 }
#define DIM_STEPS_DEFAULT_16   { 0, 31, 29, 27, 25, 23, 21, 19, 17, 15, 13, 11, 9, 7, 5, 3 }
#define DIM_STEPS_DEFAULT_32   { 0, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8, 8 }
#define DIM_STEPS_DEFAULT_64   { 0, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4 }
#define DIM_STEPS_DEFAULT_128   { 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2 }
#define DIM_STEPS_DEFAULT_256   { 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }

int dim_steps[MAX_DIM_RESOLUTION] = DIM_STEPS_DEFAULT_16;
MODULE_PARM (dim_steps, "1-" __MODULE_STRING(MAX_DIM_RESOLUTION) "1i");

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
 * globals
 */

static RT_TASK mcudev_ldim_task;
static mcu_device_t * Device;

static RTIME dim_step_length;                             // dim step length
static int dim_last_interval;

/*
 * Dimmer tables
 */

#define DIM_STEPS               256        // total number of steps


/*
 * dim tables
 *
 * contain a step index for each interval indicating when the interval
 * should begin within the step raster.
 *
 * One step is 100us.
 */

static int dim_interval_map[MAX_DIM_RESOLUTION];

/*
 * dim step table
 *
 * the dim step table gets initialized once at module startup.
 * each step contains the information to which
 * interval the step refers.
 *
 */

#define DIM_STEP_NOOP           -1
 
typedef struct dim_step {
	int interval;               // index to corresponding interval
	unsigned char port_value[PIO_MAX_PORTS]; // DEBUG: port value log
} dim_step_t;

static dim_step_t dim_step[DIM_STEPS];


/*
 * dim_interval
 *
 * the dim interval table provides per-interval data. intervals relate
 * to brightness values in reverse order: the first interval is the one
 * setting maximum brightness because its switches on the light at
 * start of the internal loop. the last interval is the one representing the off state
 * also resetting all output values because the semiconductor relay memorizes
 * the output bit until the next phase starts.
 *
 * There is one set of port values for each interval.
 * The data gets calculated at the end of every loop.
 */

typedef struct dim_interval {
	unsigned char port_value[PIO_MAX_PORTS];         // data to be set in this interval
} dim_interval_t;

static dim_interval_t dim_interval[MAX_DIM_RESOLUTION];  // table of interval related data

/*
 * setup dim step table
 *
 * Initialize the constant values of the dim step table.
 * Usually called at startup and for calibration at runtime.
 */

static void
setup_dim_step_table()
{
	int step, interval;

	/*
	 * initialize all steps to cause "no change"
	 */

	for (step = 0; step < DIM_STEPS; step++) {
		dim_step[step].interval = DIM_STEP_NOOP;
	}

	/*
	 * mark steps that activate new intervals
	 */

	for (interval = 0, step = 0;interval < dim_resolution;interval++) {
//		step += (interval == dim_last_interval ? pulse_length_steps : dim_interval_map[interval]);
		step += dim_interval_map[interval];
		dim_step[step % DIM_STEPS].interval = interval;
	}

	/*
	 * print new table content
	 */

	for (step = 0; step < DIM_STEPS;step++) {
		printk ("%s: step table: step %3d: %2d\n", MCUDEV_NAME, step, dim_step[step].interval);
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
prepare_port_values (void)
{
	int lamp, interval, port;

	/*
	 * set port values for each interval to zero
	 */

	for (interval = 0; interval < dim_resolution; interval++) {
		for (port = 0;port < PIO_MAX_PORTS;port++) {
			dim_interval[interval].port_value[port] = 0;
		}
	}

	/*
	 * set port value bit in the corresponding interval port value
	 */
	for (lamp = 0; lamp < max_lamps; lamp++) {
		if (Device->pixel[lamp][0]) {
			if (Device->pixel[lamp][0] > dim_last_interval) {
				rt_printk("%s: Illegal lamp value (lamp=%d value=%d)\n", MCUDEV_NAME, lamp, Device->pixel[lamp][0]);
				continue;
			}

			interval = dim_last_interval - Device->pixel[lamp][0];
			dim_interval[interval].port_value[lamp/8] |= (1 << (lamp % 8));
		}
	}

}

/*
 * DEBUG: dump_port_values
 */

static void
dump_port_values(void)
{
	int interval, port, bit;

	printk ("Dumping interval port values:\n");
	for (interval = 0; interval < dim_resolution; interval++) {
		printk ("interval %2d: ", interval);
		for (port = 0;port < PIO_MAX_PORTS;port++) {
			for ( bit = 0;bit < 8;bit++) {
				if ((dim_interval[interval].port_value[port] & (1 << bit)) )
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
 * Dimmer Process
 */

static void
mcudev_ldim_process (int data)
{
//	int first_sync = FALSE;
	int interval, port;
	unsigned char value;
	int current_step = 0;

	unsigned char port_value[PIO_MAX_PORTS];                 // buffer for new data to be written to the ports
	unsigned char port_mask[PIO_MAX_PORTS];                  // buffer for port mask

	unsigned char last_port_value[PIO_MAX_PORTS];            // buffer for last values being written to the ports

	rt_printk ("%s: real time task started\n", MCUDEV_NAME);

	/*
	 * initialize data structures
	 */
	prepare_port_values();

	memset(&last_port_value, 0, PIO_MAX_PORTS * sizeof(unsigned char) );

	while (TRUE) {

		if (dim_step[current_step].interval != DIM_STEP_NOOP) {
	
			/*
			 * determine current port values and port mask
			 */
		
			memset(&port_value, 0, PIO_MAX_PORTS * sizeof(unsigned char) );
			memset(&port_mask,  0, PIO_MAX_PORTS * sizeof(unsigned char) );
		
			interval = dim_step[current_step].interval;

			for (port = 0;port < PIO_MAX_PORTS;port ++) {
				if (interval != DIM_STEP_NOOP) {
					port_value[port] |= dim_interval[interval].port_value[port];
					if (interval == dim_last_interval) {
						/*
						 * force all bits to 0 in last interval (off)
						 */
						port_mask[port] |= 0xff;
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
		 * check if we switch to interval 0 and if so, prepare
		 * the port values according to the current pixel data.
		 */

		current_step = (current_step + 1) % DIM_STEPS;

		if (dim_step[current_step].interval == 0) {
			prepare_port_values();
		}

		/*
		 * wait for next period
		 */
		
		rt_task_wait_period();

	}
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
 * Set up periodic real time process for actual port handling.
 */

int
init_module(void)
{
	int status, lamp, interval;
	RTIME period, current_time;

	printk ("%s: init_module\n", MCUDEV_NAME);

 
	Device = (mcu_device_t *) rtai_kmalloc(MCU_DEVICE_SHMEM_MAGIC, sizeof(mcu_device_t));
	if (Device == NULL) {
		printk ("%s: rtai_kmalloc failed\n", MCUDEV_NAME);
		return 1;
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

	memcpy (&dim_interval_map, &dim_steps, dim_resolution * sizeof(int));

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
	 * set up dim step table
	 */
	setup_dim_step_table();

	{
		/*
		 * DEBUG: print calculated port values based on init_values
		 */
		prepare_port_values();
		dump_port_values();
	}

	/*
	 * initialize MCU PIO and IRQ subsystem
	 */
	if (init_mcu) {
		mcu_setup();
	}
	pio_setup();

	/*
	 * Initialize real time process
	 */
	status = rt_task_init(&mcudev_ldim_task, &mcudev_ldim_process, 0, 4096, 1, 0, NULL);
	if (status != 0) {
		printk("%s: rt_task_init failed (%d)\n", MCUDEV_NAME, status);
		release_memory();
		return 1;
	}

	rt_set_periodic_mode();

	period = start_rt_timer(dim_step_length);
	current_time = rt_get_time();

	status = rt_task_make_periodic(&mcudev_ldim_task, current_time + period, dim_step_length);
	if (status != 0) {
		printk("%s: rt_task_make_periodic failed (%d)\n", MCUDEV_NAME, status);
		release_memory();
		return 1;
	}


	/*
	 * report parameter setup
	 */
	printk("%s: dim_resolution=%d\n", MCUDEV_NAME, dim_resolution);
	printk("%s: dim_steps=", MCUDEV_NAME);
	for (interval = 0;interval < dim_resolution;interval++) {
		printk("[%2d] ", dim_interval_map[interval]);
	}
	printk("\n");
	printk("%s: init_value=%d\n", MCUDEV_NAME, init_value);
	printk("%s: max_lamps=%d\n", MCUDEV_NAME, max_lamps);
	printk("%s: dim_step_length_ns=%d\n",	MCUDEV_NAME, dim_step_length_ns);
	printk("%s: dim_step_length=%lld\n",	MCUDEV_NAME, dim_step_length);

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
	 * Terminate Real Time Process
	 */
	status = rt_task_delete(&mcudev_ldim_task);
	if (status != 0) {
		printk("%s: rt_task_delete failed (%d)\n", MCUDEV_NAME, status);
	}

	/*
	 * Switch off all lamps
	 */
	pio_init_ports(0x00);

	printk ("%s: cleanup_module\n", MCUDEV_NAME );

	return;
}

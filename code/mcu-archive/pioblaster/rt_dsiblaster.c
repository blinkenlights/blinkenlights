#include <linux/module.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");


#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>

#define micro2nano(x)	((1000 * (x))
#define milli2nano(x)	((1000000 * (x))

/*
 * Define Target PIO Output Pin
 */
#define PIO_CHIP 0		// Chip 0/1 (unused)
#define PIO_PORT 0		// Port 0-2
#define PIO_PIN  1		// Pins 0-7

#define PIO1_BASE_ADDRESS 0x200
#define PIO2_BASE_ADDRESS 0x280

/*
 * AMD Elan Chip Control
 */
#define CSCIR 0x22 // SC410 CSC Index Register
#define CSCDR 0x23 // SC410 CSC Data Register


/////////////////////////////////////////////////////////////////////////
// SC410 Low Level Function: windex -- Write Byte to CSC Registerspace

static void windex (unsigned char index, unsigned char data)
{
	outb (index, CSCIR);
	outb (data, CSCDR);
}

///////////////////////////////////////////////////////////////////////////
// SC410 Low Level Function: rindex -- Read Byte from CSC Registerspace
static unsigned char rindex (unsigned char index)
{
	outb (index, CSCIR);
	return (inb (CSCDR));
}


/*
 * DSI Protocol Variables
 */

#define DSI_TICK_PERIOD_NS		833333	// 833,333 us = 1200 ticks / second

static RT_TASK task;

typedef enum dsi_state {
	DSI_STATE_PAUSE,
	DSI_STATE_START,
	DSI_STATE_DATA,
	DSI_STATE_END
} dsi_state_t, DSI_STATE;

struct dsi_state_info {
	enum dsi_state state;
	enum dsi_state next_state;
	int ticks;
	unsigned char * variable_data;
	int static_data;
};

/*
 * encode_dsi_byte
 *
 * manchester encode a byte to int array
 */

static void encode_dsi_byte (unsigned char value, unsigned char frame_data[])
{
	int index;

	for (index = 0;index < 8;index++) {
		if( (value & (1 << (7 - index))) == 0) {
			// logical ZERO: 0 -> 1 transition
			frame_data[2*index+0] = 0;
			frame_data[2*index+1] = 1;
		} else {
			// logical ONE: 1 -> 0 transition
			frame_data[2*index+0] = 1;
			frame_data[2*index+1] = 0;
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

static unsigned char dim_table [24] = {
#ifdef DIM_MODE_LINEAR
	  1, 128, 163, 185, 200, 212, 222, 230, 238, 244, 250, 255,
	255, 250, 244, 238, 230, 222, 212, 200, 185, 163, 128, 1
#endif
	  1,  24,  47,  70,  93, 116, 139, 162, 185, 208, 231, 255,
	255, 231, 208, 185, 162, 139, 116,  93,  70,  47,  24,   1
};

static void dsi_blaster (int data)
{
	/* frame_data
	 * 	[ 0, 1]: manchester encoded bit 0
	 * 	[ 2, 3]: manchester encoded bit 1
	 * 	[ 4, 5]: manchester encoded bit 2
	 * 	[ 6, 7]: manchester encoded bit 3
	 * 	[ 8, 9]: manchester encoded bit 4
	 * 	[10,11]: manchester encoded bit 4
	 * 	[12,13]: manchester encoded bit 5
	 * 	[14,15]: manchester encoded bit 7
	 */

	unsigned char frame_data[16] = {
		DSI_1, DSI_0, DSI_0, DSI_0, DSI_0, DSI_0, DSI_0, DSI_0
	};

	struct dsi_state_info dsi_state [] = {
		{ DSI_STATE_PAUSE, DSI_STATE_START, 79, NULL, 0 },
		{ DSI_STATE_START,  DSI_STATE_DATA,  1, NULL, 1 },
		{  DSI_STATE_DATA,   DSI_STATE_END, 16, frame_data },
		{   DSI_STATE_END, DSI_STATE_PAUSE,  4, NULL, 0 }
	};

	int dsi_current_tick = 0;
	int dsi_global_tick = 0;
	DSI_STATE dsi_current_state = DSI_STATE_PAUSE;
	unsigned char port_data;

	unsigned char dim_value = 0x01;

	while(1) {
		// check for end of state
		if(dsi_current_tick == dsi_state[dsi_current_state].ticks) {
			// move to next state, reset tick count
			dsi_current_state = dsi_state[dsi_current_state].next_state;
			dsi_current_tick = 0;

			if(dsi_current_state == DSI_STATE_DATA) {
				// set up new frame_data
				encode_dsi_byte(dim_value, frame_data);
			}
		}

		if (dsi_state[dsi_current_state].variable_data == NULL) {
			port_data = dsi_state[dsi_current_state].static_data;
		} else {
			port_data = dsi_state[dsi_current_state].variable_data[dsi_current_tick];
		}

		outb( port_data << PIO_PIN, PIO1_BASE_ADDRESS + PIO_PORT);

		// increase tick
		dsi_current_tick += 1;

		/*
		 * do the fancy stuff
		 */

		dsi_global_tick += 1;
		if(dsi_global_tick == 2400) {
			dsi_global_tick = 0;
		}

		if( (dsi_global_tick % 100) == 0) {
			dim_value = dim_table[dsi_global_tick / 100];
		}

		rt_task_wait_period();
	}
}


/*
 * init_module
 *
 * initialise DSI or RGB thread
 */

int
init_module(void)
{
	RTIME start_time;
	RTIME tick_period = 0;
	int status;

	printk ("rt_dsiblaster: init_module\n");

	// Set DNP/1486 CS1 for I/O address space 0x200 - 0x207
	// Set DNP/1486 CS2 for I/O address space 0x280 - 0x28f
	// ====================================================

	windex (0xa6, rindex (0xa6) | 0x03); // Step 1
	windex (0xa0, rindex (0xa0) | 0x05); // Step 2
	windex (0x3b, rindex (0x3b)| 0x03); // Step 3
	windex (0xe5, (rindex (0xe5) & 0xfe) | 0x01); // Step 4
	windex (0xb4, 0x00); // CSA Step 5.1

	windex (0xb5, 0x22); // CSA Step 5.2
	windex (0xb6, 0x80); // CSB Step 5.1
	windex (0xb7, 0x02); // CSB Step 5.2
	windex (0xb8, (rindex (0xb8) & 0x88) | 0x33); // Step 6
	windex (0xb2, 0x10); // Step 7
	windex (0xa6, rindex (0xa6) & 0xfc); // Step 8


	outb( 0x80, PIO1_BASE_ADDRESS + 3);

	rt_task_init(&task, &dsi_blaster, 0, 4096, 1, 0, NULL);
//	rt_task_init(&task, &rgb_blaster, 0, 4096, 1, 0, NULL);

	rt_set_periodic_mode();
	tick_period = start_rt_timer(nano2count(DSI_TICK_PERIOD_NS));
//	tick_period = start_rt_timer(nano2count(RGB_TICK_PERIOD_NS));
	start_time = rt_get_time() + 10 * tick_period;
	status = rt_task_make_periodic(&task, start_time, tick_period);

	printk("rt_dsiblaster: timer period = %d\n", (int) tick_period);
	printk("rt_dsiblaster: 1ns = count %d\n\n", (int)nano2count(1));
	printk("rt_dsiblaster: 1us = count %d\n\n", (int)nano2count(1000));
	printk("rt_dsiblaster: 1ms = count %d\n\n", (int)nano2count(1000000));

	return 0;
}

void cleanup_module (void)
{
	rt_task_delete(&task);
	outb( 0x00, PIO1_BASE_ADDRESS + 0);
	printk ("rt_dsiblaster: cleanup_module\n");
}

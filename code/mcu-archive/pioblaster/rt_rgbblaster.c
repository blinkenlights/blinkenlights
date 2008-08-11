#include <linux/module.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");


#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>

#define micro2nano(x)	((1000 * (x))
#define milli2nano(x)	((1000000 * (x))

/*
 * program options
 */
#define INVERT_OUTPUT

#define  HSV

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


static RT_TASK task;

/*
 * RGB Protocol Variables
 */

#define RGB_TICK_PERIOD_NS		60000	// 60 us 

typedef enum rgb_state {
	RGB_STATE_PAUSE,
	RGB_STATE_GREEN,
	RGB_STATE_RED,
	RGB_STATE_BLUE
} rgb_state_t, RGB_STATE;

struct rgb_state_info {
	enum rgb_state state;
	enum rgb_state next_state;
	int ticks;
	unsigned char * variable_data;
	int static_data;
};

/*
 * encode_rgb_byte
 *
 * encode a byte to int array according to RGB protocol
 */

static void encode_rgb_byte (unsigned char value, unsigned char frame_data[])
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


#ifdef HSV

#define RES    10000
#define INT(x) ((x/RES)*RES)

static void
hsv_to_rgb (unsigned char hue, unsigned char sat, unsigned char val,
			unsigned char *red, unsigned char *green, unsigned char *blue)
{
  long h, s, v;
  long f, p, q, t;
  long r = 0, g = 0, b = 0;

  if (sat == 0)
    {
		r = val;
		g = val;
		b = val;
    }
  else
    {
      h = hue * RES * 6 / 255;
      s = sat * RES     / 255;
      v = val * RES     / 255;

      f = h - (INT(h));
      p = v * (RES - s) / RES;
      q = v * (RES - (s * f / RES)) / RES;
      t = v * (RES - (s * (RES - f) / RES)) / RES;

      switch (INT(h) / RES)
        {
        case 0:
          r = v * 255;
          g = t * 255;
          b = p * 255;
          break;

        case 1:
          r = q * 255;
          g = v * 255;
          b = p * 255;
          break;

        case 2:
          r = p * 255;
          g = v * 255;
          b = t * 255;
          break;

        case 3:
          r = p * 255;
          g = q * 255;
          b = v * 255;
          break;

        case 4:
          r = t * 255;
          g = p * 255;
          b = v * 255;
          break;

        case 5:
          r = v * 255;
          g = p * 255;
          b = q * 255;
          break;
        }
        r = r / RES;
        g = g / RES;
        b = b / RES;
    }
    
    *red   = (unsigned char) r;
    *green = (unsigned char) g;
    *blue  = (unsigned char) b;
}

#endif

/*
 * rgb_blaster
 *
 */

#define RGB_0      1,0,0,0,0        // left to right
#define RGB_1      1,1,1,1,0        // left to right

#define RGB_RED   0
#define RGB_GREEN 1
#define RGB_BLUE  2

#ifdef SETCOLOR
static unsigned char rgb_dim_table[3][16] = {
	{ 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240 },
	{ 128, 144, 160, 176, 192, 208, 224, 240, 0, 16, 32, 48, 64, 80, 96, 112 },
	{ 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240, 0, 16, 32, 48 }
};
#endif

static void rgb_blaster (int data)
{
	unsigned char rgb_frame_data[3][40] = {
		{ RGB_1, RGB_1, RGB_1, RGB_1, RGB_1, RGB_1, RGB_1, RGB_1 },
		{ RGB_0, RGB_0, RGB_0, RGB_0, RGB_0, RGB_0, RGB_0, RGB_1 },
		{ RGB_0, RGB_0, RGB_0, RGB_0, RGB_0, RGB_0, RGB_0, RGB_0 }
	};

	struct rgb_state_info rgb_state [] = {
		{ RGB_STATE_PAUSE, RGB_STATE_GREEN, 50, NULL, 0 },
		{ RGB_STATE_GREEN,   RGB_STATE_RED, 40, rgb_frame_data[RGB_GREEN] },
		{ RGB_STATE_RED,    RGB_STATE_BLUE, 40, rgb_frame_data[RGB_RED] },
		{ RGB_STATE_BLUE,  RGB_STATE_PAUSE, 40, rgb_frame_data[RGB_BLUE] },
	};

	int rgb_current_tick = 0;
	int rgb_global_tick = 0;
	RGB_STATE rgb_current_state = RGB_STATE_PAUSE;
	unsigned char port_data;

	unsigned char red_value = 255;
	unsigned char green_value = 128;
	unsigned char blue_value = 0;

	unsigned char hue = 0;
	unsigned char sat = 255;
	unsigned char val = 255;

	/*
	 * internal loop. executed every 60 us
	 */
	while(1) {
		// check for end of state
		if(rgb_current_tick == rgb_state[rgb_current_state].ticks) {
			// move to next state, reset tick count
			rgb_current_state = rgb_state[rgb_current_state].next_state;
			rgb_current_tick = 0;

			if(rgb_current_state == RGB_STATE_PAUSE) {
				// set up new frame_data
				encode_rgb_byte(red_value,   rgb_frame_data[RGB_RED]);
				encode_rgb_byte(green_value, rgb_frame_data[RGB_GREEN]);
				encode_rgb_byte(blue_value,  rgb_frame_data[RGB_BLUE]);
			}
		}

		if (rgb_state[rgb_current_state].variable_data == NULL) {
			port_data = rgb_state[rgb_current_state].static_data;
		} else {
			port_data = rgb_state[rgb_current_state].variable_data[rgb_current_tick];
		}

#ifdef INVERT_OUTPUT
		port_data = (port_data ^ 0x01);
#endif
		outb( port_data << PIO_PIN, PIO1_BASE_ADDRESS + PIO_PORT);

		// increase tick
		rgb_current_tick += 1;

		/*
		 * do the fancy stuff
		 */

		rgb_global_tick += 1;
		if(rgb_global_tick == 16000) {
			rgb_global_tick = 0;
		}

#ifdef SETCOLOR
		if( (rgb_global_tick % 1000) == 0) {
			red_value   = rgb_dim_table[RGB_RED][rgb_global_tick / 1000];
			green_value = rgb_dim_table[RGB_GREEN][rgb_global_tick / 1000];
			blue_value  = rgb_dim_table[RGB_BLUE][rgb_global_tick / 1000];
		}
#endif
#ifdef HSV
		if( (rgb_global_tick % 1000) == 0) {
			hue++;
			hsv_to_rgb(hue,sat,val,&red_value,&green_value,&blue_value);
		}
#endif
#ifdef COLORFLOW
		if( (rgb_global_tick % 1000) == 0) {
			red_value += 1;
			green_value -= 2;
			blue_value += 3;
		}
#endif

		rt_task_wait_period();
	}
}

/*
 * init_module
 *
 * initialise DSI or RGB thread
 */

int init_module(void)
{
	RTIME start_time;
	RTIME tick_period = 0;
	int status;

	printk ("rt_rgbblaster: init_module\n");

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

	rt_task_init(&task, &rgb_blaster, 0, 4096, 1, 0, NULL);

	rt_set_periodic_mode();
	tick_period = start_rt_timer(nano2count(RGB_TICK_PERIOD_NS));
	start_time = rt_get_time() + 10 * tick_period;
	status = rt_task_make_periodic(&task, start_time, tick_period);

	printk("rt_rgbblaster: timer period = %d\n", (int) tick_period);
	printk("rt_rgbblaster: 1ns = count %d\n\n", (int)nano2count(1));
	printk("rt_rgbblaster: 1us = count %d\n\n", (int)nano2count(1000));
	printk("rt_rgbblaster: 1ms = count %d\n\n", (int)nano2count(1000000));

	return 0;
}

void cleanup_module (void)
{
	rt_task_delete(&task);
	outb( 0x00, PIO1_BASE_ADDRESS + 0);
	printk ("rt_rgbblaster: cleanup_module\n");
}

/*
 * mcud - Matrix Control Unit Daemon
 *
 * mcud listens for Data Packets on a given UDP socket.
 * A MAGIC VALUE identifies each frame to be of a certain type.
 * The following types are recognised:
 *
 * 0x2342FEED     MCU Setup packet
 * 0x23542666     MCU Frame packet
 * 0x23542667     MCU Device Control packet
 * 0xDEADBEEF     Original BL Frame Packet
 *                (will be converted to MCU Frame packet)
 *
 * A MCU Setup Packet (MCU_SETUP) sets the current
 * global window matrix dimensions (rows x columns) and assigns
 * each of the MCU ports to a certain pixel coordinate.
 *
 * A MCU Frame Packet (MCU_FRAME) contains a full description for one
 * frame for the current matrix. The assignment vector sent with
 * the last MCU_SETUP packet is used to set the appropriate ports
 * to the respective values in the Frame Packet.
 *
 * A MCU Device Control packet (MCU_DEVCTRL) is used to switch on or off
 * individual lighting devices attached to the ports. This is
 * useful for lighting devices that do not switch off completely
 * when a pixel is set to its lowest pixel value (e.g. DSI controlled
 * flourescent tubes).
 *
 * The original Binkenlights Frame (BL_FRAME) will be converted on-the-fly
 * to a MCU Frame Packet and gets processed accordingly. This assures
 * compatibility to the original Blinkenlights Chaos Control Center.
 *
 *
 * Operation
 * =========
 *
 * mcud receives frame packets to be passed over to the
 * matrix driver subsystem running as a RTAI real-time process
 * in kernel mode. The matrix driver serves up to MCU_MAX_PIXELS
 * pixels connecting to the chosen lighting device. The pixels are
 * enumerated from 0 to (MCU_MAX_PIXELS - 1).
 *
 * For each pixel a three byte buffer is held in a RTAI shm Shared
 * Memory vector allowing to serve up to 24 Bit of data for the
 * pixel. The three bytes are only used in RGB mode, in GREYSCALE and
 * MONOCHROME mode only the first byte will be used.
 *
 *
 * MCU Frame Packet
 * ----------------
 *
 * mcud constantly receives MCU Frame packets on the network either
 * sent directly to its IP Address or broad/multicasted on the local subnet.
 * The packet header specifies the intended target matrix in
 * rows * columns (height * width, y * x) pixels. The matrix has its
 * origin at the upper left, so row 0 and column 0 points to the leftmost
 * pixel in the top row of the target matrix.
 *
 * A depth parameter indicates if the associated pixel value is
 * just a greyscale value (depth == 1) or a RGB value (depth ==3).
 *
 * Located after the header the data comes in a three-dimensional array
 * (pixel[row][column][depth]) with rows * colums * depth bytes length.
 *
 * The maxval parameter specifies the maximum value of each pixel value.
 * Typical maximum values are 1 (MONOCHROME), 16 or 256 (GREYSCALE/RGB),
 * but any value is possible (e.g. 2 for dual-brightness display).
 *
 *
 * MCU Target Device Settings
 * --------------------------
 *
 * All ports of a MCU represent a single device. This device is configured
 * to be in a dedicated state consisting of four parameters:
 *
 *       device_type         dsi, neon, bulb, ...
 *       device_ports        number of ports used
 *       device_depth        number of pixel values per port used
 *       device_maxval       maximum pixel values
 *
 *
 * MCU Device Types
 * ----------------
 *
 * Each MCU supports different device types. Each device type is driven
 * by a separate real-time kernel module that identifies itself with
 * a value of type mcu_device_type_t and describes its operation with
 * a value of type mcu_device_mode_t
 * 
 * Each module uses the same Shared Memory interface.
 *
 * bulb_monochrome ---
 *        lightbulb in MONOCHROME mode. It is either totally
 *        switched on or totally switched on.
 *
 * bulb_greyscale ---
 *        lightbulb in GREYSCALE mode. The bulb can be dimmed.
 *
 * bulb_rgb ---
 *        three dimmed lightbulbs forming a rgb pixel.
 * 
 * dsi_greyscale ---
 *        a DSI controlled fluorescent tube that can be dimmed and switched off.
 *
 * neon_greyscale ---
 *        a neon tube that can be dimmed and switched off
 *
 * neon_rgb ---
 *        a triple neon tube forming a rgb pixel.
 *
 * Each device performs its own adaptation to input values. For instance, a
 * DSI device automatically converts the linear input value to its
 * logarithmic dimmer curve providing a linear interface. It might also
 * prevent 0 values to be sent to the device so that a input 0 does not
 * switch off the device but displays the minimum greyscale value possible.
 *
 *
 * Pixel Depth Conversion
 * ----------------------
 *
 * mcud automatically converts different pixel value types depending on the
 * target device setup. It can convert RGB to GREYSCALE pixel values and
 * vice versa. It also adapts the maximum pixel value to the target
 * maximum value.
 *
 * For instance, a RGB packet sent to a GREYSCALE device will be
 * recalculated to express the brightness of the chosen color. Accordingly, if
 * a GREYSCALE value is sent to a RGB device, the value will be recalculated
 * to represent the same grey value in RGB.
 *
 * This might use adaptive color transformation with different factors for each
 * color (e.g. SVG conversion with red 0.2125, green 0.7154, blue 0.0721) or any
 * other device-dependent conversion table.
 *
 *
 * Pixel Resolution Conversion
 * ---------------------------
 *
 * In addition to DEPTH adaptation, the RESOLUTION is also converted depending
 * on the device's target mode. If a MONOCHROME value is applied to a 8-Bit
 * GREYSCALE device, a 0x00 will remain 0x00 but 0x01 will be converted to
 * 0xff. The other way around, any value equal or above to 0x80 will become
 * 0x01 and anything below 0x80 will become 0x00.
 *
 * Generally a maximum value <n> gets converted to maximum value <m> 
 * by the following formula
 *
 *      new_value = (unsigned char) old_value * m / n;
 *
 * This conversion provides a "good enough" apaption of frames not designed
 * for the target device retaining compatibility of sender and receiver.
 *
 *
 * Matrix Apaptation
 * -----------------
 *
 * Each MCU can be configured to support various frame sizes. For instance,
 * a 20x26 matrix can be adressed as a 20x13 matrix as well grouping two
 * pixels and displaying the same value in both pixels. In order to
 * support a different resolution, the MCU has to be reconfigured using a
 * MCU_SETUP packet before sending frames in the new matrix format.
 *
 * If a received frame size does not match the configured resolution
 * mcud cuts off (for bigger frames) or centers (for smaller frames) the image
 * to the current configuration.
 *
 *
 * Ports Setup
 * -----------
 *
 * When mcud starts up, it reads its configuration from the commandline or 
 * a configuration file so that the device is ready for operation immediately.
 * If no configuration was applied by either method, the device ignores
 * all MCU Frame packets (or legacy Blinkenlights Frame packets) until configured.
 *
 * mcud usually receives its configuration by means of a MCU Setup
 * packet (MCU_SETUP) which automatically replaces the last configuration on
 * reception of the packet. Only one configuration is valid at the same time.
 *
 * The configuration is a vector of Y/X coordinates assigning each
 * port to a certain matrix position. More than one port can point to the same
 * coordinate to allow pixel grouping. 
 *
 * <mcu_setup>
 *     <mcu-id>1</mcu-id>
 *     <height>8</height>
 *     <width>18</width>
 *     <ports>
 *         <port y="3" x="16"/>
 *         <port y="3" x="17"/>
 *         <port y="4" x="16"/>
 *         <port y="4" x="17"/>
 *         ...
 *     </ports>
 * </mcu_setup>
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/stat.h>

#include <netinet/in.h>

#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <netdb.h>

#ifdef LINUX
#include <getopt.h>
#include <libgen.h>
#endif

#ifndef LINUX
typedef int socklen_t;
#endif

#ifdef RTAI_SHARED_MEMORY
#include "rtai_shm.h"
#endif

#include "mcuprotocol.h"
#include "mcud.h"

 /*
  * miscelleanous definitions
  */

#define TRUE 1
#define FALSE 0

#define EXIT_SUCCESS     0
#define EXIT_ERROR       1
#define EXIT_DAEMON      2
#define EXIT_NO_RT_MEM   3


/*
 * Color intensity conversion. Stolen from The GIMP.
 */

#define INTENSITY_RED   0.30
#define INTENSITY_GREEN 0.59
#define INTENSITY_BLUE  0.11
#define INTENSITY(r,g,b) ((r) * INTENSITY_RED   + \
              (g) * INTENSITY_GREEN + \
              (b) * INTENSITY_BLUE  + 0.001)

/*
 * globals
 */

static char * ProgramName ="mcud";

char mcu_id = MCU_ID_ANY;       // MCU ID

char * config_file_path = NULL;

mcu_setup_t Setup;				// current device configuration

mcu_device_t * Device;			// current Device Shared Memory Area
mcu_device_t DummyDevice =  { "Default Dummy Device", TRUE, FALSE, 3, 255 };

int default_setup_height = 0;
int default_setup_width = 0;
int default_setup_depth = 0;
int default_setup_pixels = 0;

int default_device_maxval = 255;
int default_device_depth = 1;
int default_device_zero_off = FALSE;

int monitor_mode = FALSE;
int debug_mode = TRUE;          // still in development :-)


/*********************************************************/

/*
 * bl_frame operations
 */

int
bl_frame_check_size(bl_frame_t *bl_frame, int datagram_size)
{
	int required_size;

	required_size = sizeof(bl_frame_header_t) + (bl_frame->header.frame_width * bl_frame->header.frame_height);

	// ignore oversized packets
	if (required_size != datagram_size) {
		fprintf(stderr,"mcud: bl_frame_check_size: invalid datagram size (%d), must be %d\n",
			datagram_size, required_size);
		return -1;
	}
	return 0;
}


void
bl_frame_convert_byte_order(bl_frame_t *bl_frame)
{
	bl_frame->header.frame_magic = ntohl(bl_frame->header.frame_magic);
	bl_frame->header.frame_count = ntohl(bl_frame->header.frame_count);
	bl_frame->header.frame_width = ntohs(bl_frame->header.frame_width);
	bl_frame->header.frame_height = ntohs(bl_frame->header.frame_height);
}

void
bl_frame_translate(bl_frame_t *bl_frame, mcu_frame_t *mcu_frame)
{
	bl_frame_t tmp_bl_frame;
	
	tmp_bl_frame = * bl_frame;
	
	mcu_frame->header.magic  = MAGIC_MCU_FRAME;
	mcu_frame->header.height = tmp_bl_frame.header.frame_height;
	mcu_frame->header.width  = tmp_bl_frame.header.frame_width;
	mcu_frame->header.depth  = 1;
	if (tmp_bl_frame.header.frame_magic == MAGIC_BLFRAME_256) {
		mcu_frame->header.maxval = 255;
	} else {
		mcu_frame->header.maxval = 1;
	}
	memcpy(&mcu_frame->body.data, &tmp_bl_frame.body.frame_data,
		mcu_frame->header.height * mcu_frame->header.width);
}


/*
 * mcu_frame operations
 */

int
mcu_frame_check_size(mcu_frame_t * mcu_frame, int datagram_size)
{
	int required_size;

	required_size = sizeof(mcu_frame_header_t) + sizeof(u_char) *
						(mcu_frame->header.height * mcu_frame->header.width * mcu_frame->header.depth);
	if (required_size != datagram_size) {
		fprintf(stderr,"mcud: mcu_frame_check_size: invalid datagram size (%d), must be %d\n",
			datagram_size, required_size);
		return -1;
	}
	return 0;
}

void
mcu_frame_convert_byte_order(mcu_frame_t *mcu_frame)
{
	mcu_frame->header.magic = ntohl(mcu_frame->header.magic);
	mcu_frame->header.height = ntohs(mcu_frame->header.height);
	mcu_frame->header.width = ntohs(mcu_frame->header.width);
	mcu_frame->header.depth = ntohs(mcu_frame->header.depth);
	mcu_frame->header.maxval = ntohs(mcu_frame->header.maxval);
}

void
mcu_frame_write_frame_data(mcu_frame_t * mcu_frame)
{
	static int device_loaded = FALSE;
	int pixel, depth, position;
	int row, column;
	u_char * current_pixel;
	u_char device_pixel[3];

	/*
	 * Check for current configuration. Skip frame if no configuration
	 * exists or if the current matrix does not match the frame's matrix
	 */

	if (debug_mode && !monitor_mode) {
		fprintf(stderr, ".");
	}

	if (Setup.header.magic != MAGIC_MCU_SETUP) {
		fprintf(stderr, "mcud: not configured\n");
		return;
	}

	if (Device->flag_loaded == TRUE && device_loaded == FALSE) {
		fprintf(stderr, "mcud: device '%s' depth=%d  maxval=%d \n",
		      Device->name, Device->depth, Device->maxval );
		device_loaded = TRUE;
	}

	if (Device->flag_loaded == FALSE && device_loaded == TRUE) {
		device_loaded = FALSE;
		fprintf(stderr, "mcud: device not loaded\n");
		return;
	}

	if (Setup.header.height != mcu_frame->header.height
		|| Setup.header.width != mcu_frame->header.width
		|| Setup.header.depth != mcu_frame->header.depth) {
		fprintf(stderr, "mcud: Invalid Frame Matrix (%dx%dx%d) for current configuration (%dx%dx%d)\n",
			mcu_frame->header.height, mcu_frame->header.width, mcu_frame->header.depth,
			Setup.header.height, Setup.header.width, Setup.header.depth);
		return;
	}
 
	/*
	 * Write new pixel values to pixel memory
	 */

	for (pixel = 0; pixel < Setup.header.pixels; pixel++) {
		position =  Setup.body.pixel[pixel].row    * mcu_frame->header.width * mcu_frame->header.depth +
					Setup.body.pixel[pixel].column * mcu_frame->header.depth;
		current_pixel = (u_char *) &mcu_frame->body.data + position;


		/*
		 *Adapt pixel value to device depth
		 */
		switch (Device->depth) {
			case 1:
				if (mcu_frame->header.depth == 3) {
					device_pixel[0] = INTENSITY(current_pixel[0], current_pixel[1], current_pixel[2]);
				} else {
					device_pixel[0] = current_pixel[0];
				}
				break;

			case 3:
				if (mcu_frame->header.depth == 1) {
					device_pixel[0] = current_pixel[0];
					device_pixel[1] = current_pixel[0];
					device_pixel[2] = current_pixel[0];
				} else {
					device_pixel[0] = current_pixel[0];
					device_pixel[1] = current_pixel[1];
					device_pixel[2] = current_pixel[2];
				}
				break;

			default:
				fprintf(stderr, "mcud: invalid device depth (%d)\n", Device->depth);
				break;
		}

		/*
		 * Adapt maximum values
		 *
		 * TIME FOR CHANGE: This should be done with a lookup table for performance reasons
		 */
		
		if (mcu_frame->header.maxval != Device->maxval) {
			for (depth = 0;depth < Device->depth; depth++) {
				device_pixel[depth] = device_pixel[depth] * Device->maxval /  mcu_frame->header.maxval;
			}
		}

		/*
		 * Adjust 0 zero pixel value when Zero Off Flag is set
		 * to prevent the device from switching off.
		 */
		if (Device->flag_zero_off) {
			for (depth = 0;depth < Device->depth; depth++) {
				if (device_pixel[depth] == 0) {
					device_pixel[depth] = 1;
				}
			}
		}

		/*
		 * write new values atomically to prevent
		 * race conditions with illegal values
		 */
		Device->pixel[pixel][0] = device_pixel[0];
		Device->pixel[pixel][1] = device_pixel[1];
		Device->pixel[pixel][2] = device_pixel[2];
	}

	if (monitor_mode) {
		fprintf(stderr, "\033[H\033[J");		// VT100 Clear Screen

		/*
		 * print received frame data
		 */
		for (row = 0; row < mcu_frame->header.height; row++) {
			for (column = 0; column < mcu_frame->header.width; column++) {
				position =  row * mcu_frame->header.width * mcu_frame->header.depth +
					column * mcu_frame->header.depth;
				current_pixel = (u_char *) &mcu_frame->body.data + position;

				for (depth = 0;depth < mcu_frame->header.depth; depth++) {
					fprintf(stderr, "%02X", current_pixel[depth]);
				}
				fprintf(stderr, " ");
			}
			fprintf(stderr, "\n");
		}
		fprintf(stderr, "\n");

		/*
		 * print written pixel data
		 */

		for (pixel = 0; pixel < Setup.header.pixels; pixel++) {
			for (depth = 0;depth < Device->depth; depth++) {
				fprintf(stderr, "%02X", Device->pixel[pixel][depth]);
			}
			fprintf(stderr, " ");
			if ( (pixel+1) % 8 == 0) {
				fprintf(stderr, "\n");
			}
		}
		fflush(stderr);
	}


}

/*
 * mcu_setup operations
 */

int
mcu_setup_check_size(mcu_setup_t * mcu_setup, int datagram_size)
{
	int required_size;

	required_size = sizeof(mcu_setup_header_t) + (mcu_setup->header.pixels * sizeof(mcu_setup_pixel_t));
	if (required_size != datagram_size) {
		fprintf(stderr,"mcud: mcu_setup_check_size: invalid datagram size (%d), must be %d\n",
			datagram_size, required_size);
		return -1;
	}
	return 0;
}

void
mcu_setup_convert_byte_order(mcu_setup_t *mcu_setup)
{
	mcu_setup->header.magic  = ntohl(mcu_setup->header.magic);
	mcu_setup->header.height = ntohs(mcu_setup->header.height);
	mcu_setup->header.width  = ntohs(mcu_setup->header.width);
	mcu_setup->header.depth  = ntohs(mcu_setup->header.depth);
	mcu_setup->header.pixels  = ntohs(mcu_setup->header.pixels);
}

int
mcu_setup_set_config(mcu_setup_t *mcu_setup)
{
	if (mcu_setup->header.mcu_id != -1 && mcu_setup->header.mcu_id != mcu_id) {
		fprintf(stderr,"mcud: mcu_setup_set_config: ignoring mcu setup with id %d\n",
			mcu_setup->header.mcu_id);
		return -1;
	}
	Setup = * mcu_setup;
	fprintf(stderr,"mcud: mcu_setup_set_config: received new setup (%dx%dx%d)\n",
		mcu_setup->header.height, mcu_setup->header.width, mcu_setup->header.depth);
	return 0;
}


/*
 * mcu_devctrl operations
 */

int
mcu_devctrl_check_size(mcu_devctrl_t * mcu_devctrl, int datagram_size)
{
	int required_size;

	required_size = sizeof(mcu_devctrl_header_t) + (mcu_devctrl->header.pixels * sizeof(u_char));
	if (required_size != datagram_size) {
		fprintf(stderr,"mcud: mcu_devctrl_check_size: invalid datagram size (%d), must be %d\n",
			datagram_size, required_size);
		return -1;
	}
	return 0;
}

void
mcu_devctrl_convert_byte_order(mcu_devctrl_t *mcu_devctrl)
{
	mcu_devctrl->header.magic = ntohl(mcu_devctrl->header.magic);
	mcu_devctrl->header.pixels = ntohs(mcu_devctrl->header.pixels);
}

void
mcu_devctrl_set_ctrl(mcu_devctrl_t *mcu_devctrl)
{
	// do nothing so far
}


/*
 * setup_configuration
 *
 * Set up defaults. To be replaced with reading a
 * XML based configuration file in the final version.
 *
 * For now, initialize the Device and Setup data structures
 * with the default parameters allowing immediate operation.
 */

void
setup_default_configuration(void)
{
	int row, column, pixel;

	Device = &DummyDevice;

	Device->depth = default_device_depth;
	Device->maxval = default_device_maxval;
	Device->flag_zero_off = default_device_zero_off;
	Device->flag_loaded = TRUE;

	Setup.header.magic  = MAGIC_MCU_SETUP;

	Setup.header.height = default_setup_height;
	Setup.header.width  = default_setup_width;
	Setup.header.depth  = default_setup_depth;
	Setup.header.pixels = default_setup_pixels;

	for (pixel = 0, row = 0; row < Setup.header.height && pixel < Setup.header.pixels; row++) {
		for ( column = 0; column < Setup.header.width && pixel < Setup.header.pixels; column++, pixel++) {
			Setup.body.pixel[pixel].row = row;
			Setup.body.pixel[pixel].column = column;
		}
	}

	if (config_file_path != NULL) {
		fprintf (stderr, "mcud: reading setup from %s\n", config_file_path);
		process_config_file (config_file_path, &Setup);
	}
}

/*
 * main
 *
 * parse command line, setup listening socket and enter packet
 * reception loop. dispatch to appropriate handler for each packet
 * type
 */

/*
 * define these variables outside of main to prevent
 * occurence of mysterious evil bug (tm)
 */
struct sockaddr_in local_address;
struct sockaddr remote_address;
socklen_t remote_address_length;

int
main (int argc, char *argv[])
{
	int status;
	int value;

	int listener;
	fd_set readable_fds;
	struct timeval timeout;
	
	union {
		u_int32_t magic;
		bl_frame_t bl_frame;
		mcu_frame_t mcu_frame;
		mcu_setup_t mcu_setup;
		mcu_devctrl_t mcu_devctrl;
	} datagram;
	int datagram_size;

	const char short_options[] = "c:?h:w:d:p:Vi:DM";
#ifdef LONG_OPTIONS
	const struct option long_options[] = {
		{ "help",        no_argument,       NULL, '?'},
		{ "version",     no_argument,       NULL, 'V'},
		{ "config",      required_argument, NULL, 'c'},
		{ "id",          required_argument, NULL, 'i'},
		{ "height",      required_argument, NULL, 'h'},
		{ "width",       required_argument, NULL, 'w'},
		{ "depth",       required_argument, NULL, 'd'},
		{ "pixels",      required_argument, NULL, 'p'},
//		{ "maxval",      required_argument, NULL, 'v'},
//		{ "zero-off",    no_argument,       NULL, 'z'},
		{ "monitor",     no_argument,       NULL, 'M'},
		{ "debug",       no_argument,       NULL, 'D'},
		{ NULL,          0,                 NULL, 0  }
	};
#endif
	char optchar;

	/*
	 * parse command line
	 */

#ifdef BASENAME
	ProgramName = strdup (basename (argv[0]));
	if (ProgramName == NULL) {
		fprintf(stderr, "mcud: can't copy program name\n");
		exit(EXIT_ERROR);
	}
#endif

	do {
#ifdef LONG_OPTIONS
		optchar = getopt_long (argc, argv, short_options, long_options, NULL);
#else
		optchar = getopt (argc, argv, short_options);
#endif
		if (optchar == -1)
			break;
		switch (optchar) {
			case 'V':
				printf("mcud 0.1\n" );
				break;
			case '?':
				printf("Usage: %s [-hvd]\n", ProgramName);
				exit(EXIT_SUCCESS);
				break;

			case 'i':
				mcu_id = atoi(optarg);
				break;

			case 'c':
				config_file_path = strdup(optarg);
				break;

			case 'h':
				default_setup_height = atoi(optarg);
				break;
			case 'w':
				default_setup_width = atoi(optarg);
				break;
			case 'd':
				default_setup_depth = atoi(optarg);
				break;
			case 'p':
				default_setup_pixels = atoi(optarg);
				break;

			case 'M':
				monitor_mode = TRUE;
				break;
			case 'D':
				debug_mode = TRUE;
				break;
		}
    } while(TRUE);


	setup_default_configuration();

#ifdef RTAI_SHARED_MEMORY

	/*
	 * create MCU shared memory region
	 */

	Device = (mcu_device_t *) rtai_malloc((unsigned long) MCU_DEVICE_SHMEM_MAGIC, sizeof(mcu_device_t));
	if (Device == NULL) {
		fprintf(stderr, "mcud: rtai_malloc failed\n");
		exit(EXIT_NO_RT_MEM);
	}

#endif

	/*
	 * create listening socket
	 */

	listener = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (listener == -1) {
		perror("mcud: can't create listening socket");
		exit(1);
	}

	// enable reuse of local socket address (server port number)
	value = 1;
	status = setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &value, sizeof(value));
	if (status == -1) {
		perror("mcud: can't set socket option");
		exit(1);
	}

	local_address.sin_family = PF_INET;
	local_address.sin_port = htons (MCU_LISTENER_PORT);
	local_address.sin_addr.s_addr = INADDR_ANY;

	status = bind (listener, (struct sockaddr *) &local_address, sizeof(local_address));
	if (status == -1) {
		perror("mcud: can't bind local address");
		exit(1);
	}

#ifdef NONBLOCKING	
	status = fcntl(listener, F_SETFL, O_NONBLOCK);
	if (status == -1) {
		perror("mcud: can't set non-blocking mode on socket");
		exit(1);
	}
#endif

	/*
	 * everything has been set up just fine. now
	 * detach as daemon if not in debug mode
	 */

	if (!debug_mode) {
		status = daemon(0,0);
		if (status == -1) {
			perror("mcud: can't detach from terminal");
			exit(EXIT_DAEMON);
		}
	}


	/*
	 * MAIN LOOP
	 *
	 * Wait for new packets to arrive and dispatch them
	 * according to its type.
 	*/

#define SELECT_TIMEOUT 60

	fprintf(stderr, "mcud: default setup (height=%d width=%d depth=%d)\n", Setup.header.height, Setup.header.width, Setup.header.depth );
	fprintf(stderr, "mcud: ready to receive on port %d\n", MCU_LISTENER_PORT);

	while (TRUE) {
		// set up socket for select(2) call
		FD_ZERO(&readable_fds);
		FD_SET(listener, &readable_fds);

		// set up timeout
		timeout.tv_sec  = SELECT_TIMEOUT;
		timeout.tv_usec = 0;

		// wait for activity
		status = select(listener + 1, &readable_fds, NULL, NULL, &timeout);
		if (status == -1) {
			perror("mcud: select failed");
			exit(1);
		}
		if (status == 0) {
			fprintf(stderr, "mcud: timeout occured on select(2) call\n");
			continue;
		}

		status = recvfrom(listener, &datagram, (size_t) sizeof(datagram), 0, &remote_address, &remote_address_length);
		if (status == -1) {
			perror("mcud: recvfrom failed");
			fprintf(stderr, "%d %lx %ld %d %lx %lx\n",
			     listener, (unsigned long) &datagram, (long) sizeof(datagram), 0,
			     (unsigned long) &remote_address, (unsigned long) &remote_address_length);
			exit(1);
		}

		datagram_size = status;

//		fprintf(stderr,"mcud: received %d bytes, magic=0x%08lx\n", status, (long) ntohl(datagram.magic) );

		/*
		 * Process packet
		 *
		 * BL_FRAME     Pass frame data to device
		 * MCU_FRAME    Pass frame data to device
		 * MCU_DEVCTRL  Set device control data
		 * MCU_SETUP    Set new configuration table
		 */

		switch(ntohl(datagram.magic)) {
			case MAGIC_BLFRAME:
			case MAGIC_BLFRAME_256:
				bl_frame_convert_byte_order(&datagram.bl_frame);
				status = bl_frame_check_size(&datagram.bl_frame, datagram_size);
				if (status == -1)
					break;
				bl_frame_translate(&datagram.bl_frame, &datagram.mcu_frame);
				mcu_frame_write_frame_data(&datagram.mcu_frame);
				break;

			case MAGIC_MCU_FRAME:
				mcu_frame_convert_byte_order(&datagram.mcu_frame);
				status = mcu_frame_check_size(&datagram.mcu_frame, datagram_size);
				if (status == -1)
					break;
				mcu_frame_write_frame_data(&datagram.mcu_frame);
				break;

			case MAGIC_MCU_SETUP:
				mcu_setup_convert_byte_order(&datagram.mcu_setup);
				status = mcu_setup_check_size(&datagram.mcu_setup, datagram_size);
				if (status == -1)
					break;
				if (datagram.mcu_setup.header.height * datagram.mcu_setup.header.width > MCU_MAX_PIXELS) {
					fprintf(stderr,"mcud: setup dimensions (%d/%d) exceed maximum matrix width (%d)\n",
						datagram.mcu_setup.header.height, datagram.mcu_setup.header.width, MCU_MAX_PIXELS);
				}
				mcu_setup_set_config(&datagram.mcu_setup);
				break;

			case MAGIC_MCU_DEVCTRL:
				mcu_devctrl_convert_byte_order(&datagram.mcu_devctrl);
				status = mcu_devctrl_check_size(&datagram.mcu_devctrl, datagram_size);
				if (status == -1)
					break;
				mcu_devctrl_set_ctrl(&datagram.mcu_devctrl);
				break;
		}
	}


	/*
	 * cleanup
	 */

#ifdef RTAI_SHARED_MEMORY

	/*
	 * free MCU shared memory region
	 */

	rtai_free(MCU_DEVICE_SHMEM_MAGIC, Device);

#endif
	
	status = close(listener);
	if (status == -1) {
		perror("mcud: can't close socket");
		exit(1);
	}

	exit (0);
}


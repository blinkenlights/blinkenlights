/*
 * sendframes
 *
 * sendframes is a small utility sending out
 * random pixel values for a target window matrix
 * with configurable dimensions.
 *
 * Before sendframes starts sending MCU Frames it sends
 * out a MCU Configuration Packet to set the MCU in the
 * corresponding mode. No special assignment is made. Each
 * port is assigned the next pixel in the matrix starting in
 * the top left corner proceedings left-to-right and top-down
 * respectively.
 *
 * Usage:
 *
 *     sendframes [options] host ...
 *
 * Options:
 *
 *     --width <width>             Number of horizontal pixels
 *     --height <height>           Number of vertical pixels
 *     --depth <depth>             Number of values per pixel
 *     --maxval <maxval>           Maximum pixel values
 *     --pixels <pixels>           Number of pixels
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


#include "mcud.h"

 /*
  * miscelleanous definitions
  */

#define TRUE  1
#define FALSE 0

#define EXIT_SUCCESS  0
#define EXIT_ERROR    1

/*
 * globals
 */

char * program_name ="mcud";

/*
 * main
 *
 * parse command line, setup listening socket and enter packet
 * reception loop. dispatch to appropriate handler for each packet
 * type
 */
int
main (int argc, char *argv[])
{
	int status;
	struct sockaddr_in address;
	int debug_mode = FALSE;
	int configured = FALSE;
	int sender;

	int width = 18;
	int height = 8;
	int depth  = 1;
	int maxval = 1;
	int pixels  = MCU_MAX_PIXELS;

	mcu_frame_t frame;
	unsigned char * frame_data;

	struct hostent * target_host;


	/*
	 * parse command line
	 */

	const char short_options[] = "hvDxyd";
#ifdef LONG_OPTIONS
	const struct option long_options[] = {
		{ "help",        no_argument,       NULL, 'h'},
		{ "version",     no_argument,       NULL, 'V'},
		{ "debug",       no_argument,       NULL, 'D'},
		{ "width",       required_argument, NULL, 'x'},
		{ "height",      required_argument, NULL, 'y'},
		{ "maxval",      required_argument, NULL, 'r'},
		{ "pixels",      required_argument, NULL, 'p'},
		{ NULL,          0,                 NULL, 0  }
	};
#endif

	char optchar;

#ifdef BASENAME
	program_name = strdup (basename (argv[0]));
	if(program_name == NULL) {
		fprintf(stderr, "sendframes: can't copy program name");
		exit(EXIT_ERROR);
	}
#endif

	do {
#ifdef LONG_OPTIONS
		optchar = getopt_long (argc, argv, short_options, long_options, NULL);
#else
		optchar = getopt (argc, argv, short_options);
#endif
		if(optchar == -1)
			break;
		switch (optchar) {
			case 'x':
				width = atoi(optarg);
				break;
			case 'y':
				height = atoi(optarg);
				break;
			case 'm':
				maxval = atoi(optarg);
				break;
			case 'V':
				fprintf(stderr, "sendframes 0.1\n" );
				break;
			case 'h':
				fprintf(stderr, "Usage: %s [-hvd]\n", program_name);
				exit(EXIT_SUCCESS);
				break;
			case 'D':
				debug_mode = TRUE;
				break;
		}
    } while(TRUE);

	/*
	 * set up frame data buffer
	 */
	
	frame_data = calloc( width * height, sizeof(unsigned char));
	if(frame_data == NULL) {
		fprintf(stderr, "sendframes: can't allocate frame data buffer\n");
		exit(EXIT_ERROR);
	}

	/*
	 * Retrieve target host address
	 */

	if(optind >= argc) {
		fprintf(stderr, "sendframes: no target host specified\n");
		exit(EXIT_ERROR);
	}

	target_host = gethostbyname(argv[optind]);
	if(target_host == NULL) {
		fprintf(stderr, "sendframes: can't resolve hostname '%s'\n", argv[optind]);
		exit(EXIT_ERROR);
	}

	/*
	 * set up target address
	 */
	address.sin_family = PF_INET;
	address.sin_port = htons (MCU_LISTENER_PORT);
	memcpy(&address.sin_addr, &target_host->h_addr_list[0], sizeof (struct in_addr));

	/*
	 * create socket
	 */

	sender = socket( PF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if(sender == -1) {
		perror("sendframes: socket: can't create listening socket");
		exit(1);
	}

	/*
	 * MAIN LOOP
	 *
     * Continuously send MCU Frames. Send initial MCU Configuration first.
 	*/

	while (TRUE) {
		if (!configured) {
			/*
			 * set up and send MCU Configuration Packet
			 */
		}

		/*
		 * set up and send MCU Frame Packet
		 */

		status = sendto(sender, &frame, sizeof(frame), 0, &address, sizeof(address));                   
		if(sender == -1) {
			perror("sendframes: sendto: can't send MCU Frame with");
			exit(1);
		}

		sleep(1);		// wait a bit
	}


	/*
	 * cleanup
	 */
	
	status = close(sender);
	if (status == -1) {
		perror("sendframes: can't close socket");
		exit(1);
	}

	free(frame_data);

	exit (0);
}




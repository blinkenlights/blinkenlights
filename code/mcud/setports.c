/*
 * setports.c
 *
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/io.h>

#include "pio.h"

/* set PIO defaults */

int pio_chip_addr[PIO_MAX_CHIPS] = PIO_CHIP_ADDR;
int pio_port_addr[PIO_MAX_PORTS];

/*
 * Initialize PIO chips to output mode
 */
void
pio_setup(void)
{
	int chip, port;

	/*
	 * Initialize port address array
	 */
	for (chip = 0, port = 0; chip < PIO_MAX_CHIPS; chip++, port += 3) {
		pio_port_addr[port+0] = pio_chip_addr[chip] + PIO_PORT_A;
		pio_port_addr[port+1] = pio_chip_addr[chip] + PIO_PORT_B;
		pio_port_addr[port+2] = pio_chip_addr[chip] + PIO_PORT_C;
	}
}

/*
 * pio_init_ports
 * 
 * utility routine to initialize all I/O ports in one go.
 * returns the time neede to write the data
 */

void
pio_init_ports (unsigned char value)
{
	int chip, port, j;

	/*
	 * Set all PIO Ports to Mode 0 and all Pins to Output
	 */
	for (chip = 0; chip < PIO_MAX_CHIPS; chip++) {
		outb( 0x80, pio_chip_addr[chip] + PIO_CONTROL);
	}
        
	for (j = 0, value = 0; j < 8; j++) {
          value |= (1 << j);
	  for(port = 0; port < PIO_MAX_PORTS; port++) {

		printf("Writing Port %d at address %x with value %d\n", port, pio_port_addr[port], value);

		  outb(value, pio_port_addr[port]);
                  
		}

           getchar();
 	}
}

int
main (int argc, char * argv[])
{
	int chip, status;
	unsigned char value = 0xff;


	if (argc > 1) {
		value = atoi (argv[1]);
	}

	printf("Initializing PIO ports\n");
	pio_setup();

	for ( chip = 0;chip < PIO_MAX_CHIPS;chip++) {
		status = ioperm ( pio_chip_addr[chip], 4, 1);
		if (status < 0) {
			printf ("ioperm returned %d.\n", status);
			return 1;
		}
	}

	printf("Setting PIO ports\n");
	pio_init_ports(value);

	printf("Done.\n");

	return 0;
}

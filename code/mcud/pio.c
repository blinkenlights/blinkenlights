/*
 * pio.c
 *
 * PIO utility routines
 */

#include <sys/io.h> /* for glibc */

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

	/*
	 * Set all PIO Ports to Mode 0 and all Pins to Output
	 */
	for (chip = 0; chip < PIO_MAX_CHIPS; chip++) {
		outb( 0x80, pio_chip_addr[chip] + PIO_CONTROL);
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
	int port;

	for(port = 0;port < PIO_MAX_PORTS;port++) {
		outb(value, pio_port_addr[port]);
	}
}

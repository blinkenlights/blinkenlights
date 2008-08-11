/*
 * mcu.c
 *
 * MCU setup utilty routines
 */

#include <asm/io.h>

#include "mcu.h"

/*
 * PIO IO base addresses
 */

int pio_port_io_addr[MCU_PORTS] = {
	PIO_A_BASE + PIO_PORT_A,
	PIO_A_BASE + PIO_PORT_B,
	PIO_A_BASE + PIO_PORT_C,
	PIO_B_BASE + PIO_PORT_A,
	PIO_B_BASE + PIO_PORT_B,
	PIO_B_BASE + PIO_PORT_C
};

/*
 * SC410 Low Level Function: csc_write -- Write Byte to CSC Registerspace
 */

void
csc_write (unsigned char index, unsigned char data)
{
	outb (index, CSCIR);
	outb (data, CSCDP);
}


/*
 * SC410 Low Level Function: csc_read -- Read Byte from CSC Registerspace
 */

unsigned char
csc_read (unsigned char index)
{
	outb (index, CSCIR);
	return (inb (CSCDP));
}


/*
 * Initialize MCU PIO chips
 */
void mcu_setup_pio(void)
{
	// Set DNP/1486 CS1 for I/O address space 0x200 - 0x207
	// Set DNP/1486 CS2 for I/O address space 0x280 - 0x28f
	// ====================================================

	/*
	 * Set CS1 and CS2 output to logical 1
	 */
	csc_write (GPIO_RBWR_A,
		csc_read (GPIO_RBWR_A) | (GP0STAT_CTL | GP1STAT_CTL));

	/*
	 * Define CS1 and CS2 as output
	 */
	csc_write (GPIO_CS_FSR_A,
		csc_read (GPIO_CS_FSR_A) | (CS0_DIR | CS1_DIR) );

	/*
	 * Enable internal pull-up for CS1 and CS2
	 */
	csc_write (GPIO_TCR_A,
		csc_read (GPIO_TCR_A) | (CS0_PUEN | CS1_PUEN) );

	/*
	 * Enable pull-ups with latch impulse
	 */
	csc_write (SMPSOR,
		( csc_read (SMPSOR) & ~TERM_LATCH ) | TERM_LATCH);

	/*
	 * Set PIO A IO Base Address and 4-Byte Address Mask
	 */
	csc_write (GP_CSA_IO_ADR,  (PIO_A_BASE & 0xff) );
	csc_write (GP_CSA_IO_ADMR, ((PIO_A_BASE >> 8) & 0x3) | CSA_SA2_MASK | CSA_SA3_MASK );

	/*
	 * Set PIO B IO Base Address and 4-Byte Address Mask
	 */
	csc_write (GP_CSB_IO_ADR,  (PIO_B_BASE & 0xff) );
	csc_write (GP_CSB_IO_ADMR, ((PIO_B_BASE >> 8) & 0x3) | CSB_SA2_MASK | CSB_SA3_MASK );

	/*
	 * Set CSA and CSB to be additionally qualified
	 * by either IOR or IOW.  Set bus size to 8 bit.
	 */
	csc_write (GP_CSAB_IO_CQR, (csc_read (GP_CSAB_IO_CQR) & 0x88) |
		CSA_GATED_IOX0 | CSA_GATED_IOX1 | CSB_GATED_IOX0 | CSB_GATED_IOX1);

	/*
	 * Map GP_CSA to GPIO_CS0
	 * Map GP_CSB to GPIO_CS1
	 */
	csc_write (GP_CS_GPIO_CS_MRA, 0x10);

	/*
	 * Let CS1 and CS2 be driven by CSA and CSB now
	 */
	csc_write (GPIO_RBWR_A,
		csc_read (GPIO_RBWR_A) & ~(GP0STAT_CTL|GP1STAT_CTL));

	/*
	 * Set PIO1 and PIO1 to Mode 0 and all Pins to Output
	 */
	outb( 0x80, PIO_A_BASE + 3);
	outb( 0x80, PIO_B_BASE + 3);
}

/*
 * mcu_set_io_ports
 * 
 * utility routine to initialize all I/O ports in one go.
 * returns the time neede to write the data
 */

void
mcu_set_io_ports(unsigned char value)
{
	int port;

	for(port = 0;port < MCU_PORTS;port++) {
		outb(value, pio_port_io_addr[port]);
	}
}

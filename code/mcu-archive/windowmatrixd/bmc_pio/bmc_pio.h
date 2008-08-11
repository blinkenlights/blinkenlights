/*
 * bmc_pio - BMC PIO card driver for Linux 2.2
 */

#ifndef _BMC_PIO_H_
#define _BMC_PIO_H_

// CONFIG

#undef	DEBUG_IOCTLS
#undef	DEBUG_FOPS
#define DEBUG_MISC		1

// max number of chips
#define BMC_PIO_MAXCHIPS	8

// major number for all devices
#define BMC_PIO_MAJOR		60

// NO USER SERVICABLE CODE BELOW

// out ioctl magic
#define BMC_PIO_IOMAGIC		'B'

// io directions for the _DIRECTION ioctl
#define BMC_PIO_OUTPUT		0
#define BMC_PIO_INPUT		1

// our beloved ioctls
#define BMC_PIO_CTL_DIRECTION	_IOW (BMC_PIO_IOMAGIC, 0, char)
#define BMC_PIO_CTL_SETMODE_A	_IOW (BMC_PIO_IOMAGIC, 1, char)
#define BMC_PIO_CTL_SETMODE_B	_IOW (BMC_PIO_IOMAGIC, 2, char)

// the port flags
#define BMC_PIO_FLAG_WRLOCK	0x01

// port selection
#define BMC_PIO_PORT_A		0
#define BMC_PIO_PORT_B		1
#define BMC_PIO_PORT_C		2
#define BMC_PIO_PORT_CTL	3

// bitmasks for the control port
#define	BMC_PIO_GROUPA_MALL	0x60
#define BMC_PIO_GROUPA_M0	0x00
#define BMC_PIO_GROUPA_M1	0x20
#define BMC_PIO_GROUPA_M2	0x40
#define BMC_PIO_GROUPB_MALL	0x04
#define BMC_PIO_GROUPB_M0	0x00
#define BMC_PIO_GROUPB_M1	0x04
#define BMC_PIO_PORTA_IN	0x10
#define BMC_PIO_PORTB_IN	0x02
#define BMC_PIO_PORTCL_IN	0x01
#define BMC_PIO_PORTCU_IN	0x08
#define BMC_PIO_MODESET		0x80

// get the chip number for the minor number of a device
#define BMC_PIO_CHIP(minor)	( minor/3 )
#define BMC_PIO_PORT(minor)	( minor%3 )

// the default mode. both groups in mode 0, all ports configured as inputs
#define BMC_PIO_DEFCTL		(BMC_PIO_MODESET|BMC_PIO_GROUPA_M0|BMC_PIO_GROUPB_M0|BMC_PIO_PORTA_IN|BMC_PIO_PORTB_IN|BMC_PIO_PORTCL_IN|BMC_PIO_PORTCU_IN)
#define BMC_PIO_DEFFLAGS	0x00

/* !_BMC_PIO_H_*/
#endif

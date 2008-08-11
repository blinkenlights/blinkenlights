/* pio.h
 *
 * definitions for PIO operation
 */

#ifndef _PIO_H_
#define _PIO_H_


/*******************************************************************
 * 8255 PIO CHIPS
 */

/*
 * define 8255 specific port addresses
 */

#define PIO_PORT_A     0x00
#define PIO_PORT_B     0x01
#define PIO_PORT_C     0x02
#define PIO_CONTROL    0x03


#ifdef SYSTEM_MCU

/*
 * SYSTEM: Blinkenlights MCU
 */

#define PIO_MAX_CHIPS	  2
#define PIO_CHIP_ADDR     { 0x0200, 0x0280 }

#endif /* SYSTEM_MCU */


#ifdef SYSTEM_DECISION

/*
 * SYSTEM: PC with Decision Computer 192 Ports
 */

#define PIO_MAX_CHIPS      8
/*
#define PIO_CHIP_ADDR     { 0x0100, 0x0104, 0x0108, 0x010C, 0x0110, 0x0114, 0x0118, 0x011C } 
#define PIO_CHIP_ADDR     { 0x0140, 0x0144, 0x0148, 0x014C, 0x0150, 0x0154, 0x0158, 0x015C }
#define PIO_CHIP_ADDR     { 0x0280, 0x0284, 0x0288, 0x028C, 0x0290, 0x0294, 0x0298, 0x029C }
#define PIO_CHIP_ADDR     { 0x0300, 0x0304, 0x0308, 0x030C, 0x0310, 0x0314, 0x0318, 0x031C }
*/
#define PIO_CHIP_ADDR     { 0x0100, 0x0104, 0x0108, 0x010C, 0x0110, 0x0114, 0x0118, 0x011C } 

#endif /* SYSTEM_DECISION*/




#define PIO_MAX_PORTS		(PIO_MAX_CHIPS * 3)

extern int pio_chip_addr[PIO_MAX_CHIPS];
extern int pio_port_addr[PIO_MAX_PORTS];

extern void pio_setup (void);
extern void pio_init_ports (unsigned char value);


#endif /* _PIO_H_ */

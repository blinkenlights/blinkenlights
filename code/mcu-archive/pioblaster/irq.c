#include <linux/module.h>
#include <asm/io.h>

MODULE_LICENSE("GPL");

#include <rtai.h>
#include <rtai_sched.h>
#include <rtai_fifos.h>
#include <rtai_shm.h>

#include "sc410.h"

#define PHASE_IRQ    14

#define EXIT_SUCCESS  0
#define EXIT_ERROR    1

/////////////////////////////////////////////////////////////////////////
// SC410 Low Level Function: windex -- Write Byte to CSC Registerspace

static void windex (unsigned char index, unsigned char data)
{
	outb (index, CSCIR);
	outb (data, CSCDP);
}

///////////////////////////////////////////////////////////////////////////
// SC410 Low Level Function: rindex -- Read Byte from CSC Registerspace
static unsigned char rindex (unsigned char index)
{
	outb (index, CSCIR);
	return (inb (CSCDP));
}

/*
 * Initialize MCU PIO chips
 */

#define PIO0_BASE 0x0200
#define PIO1_BASE 0x0280

void mcu_setup_pio(void)
{
	// Set DNP/1486 CS1 for I/O address space 0x200 - 0x207
	// Set DNP/1486 CS2 for I/O address space 0x280 - 0x28f
	// ====================================================

	/*
	 * Set CS1 and CS2 output to logical 1
	 */
	windex (GPIO_RBWR_A,
		rindex (GPIO_RBWR_A) | (GP0STAT_CTL | GP1STAT_CTL));

	/*
	 * Define CS1 and CS2 as output
	 */
	windex (GPIO_CS_FSR_A,
		rindex (GPIO_CS_FSR_A) | (CS0_DIR | CS1_DIR) );

	/*
	 * Enable internal pull-up for CS1 and CS2
	 */
	windex (GPIO_TCR_A,
		rindex (GPIO_TCR_A) | (CS0_PUEN | CS1_PUEN) );

	/*
	 * Enable pull-ups with latch impulse
	 */
	windex (SMPSOR,
		( rindex (SMPSOR) & ~TERM_LATCH ) | TERM_LATCH);

	/*
	 * Set PIO0 IO Base Address and 4-Byte Address Mask
	 */
	windex (GP_CSA_IO_ADR,  (PIO0_BASE & 0xff) );
	windex (GP_CSA_IO_ADMR, ((PIO0_BASE >> 8) & 0x3) | CSA_SA2_MASK | CSA_SA3_MASK );

	/*
	 * Set PIO1 IO Base Address and 4-Byte Address Mask
	 */
	windex (GP_CSB_IO_ADR,  (PIO1_BASE & 0xff) );
	windex (GP_CSB_IO_ADMR, ((PIO1_BASE >> 8) & 0x3 | CSB_SA2_MASK | CSB_SA3_MASK ));

	/*
	 * Set CSA and CSB to be additionally qualified
	 * by either IOR or IOW.  Set bus size to 8 bit.
	 */
	windex (GP_CSAB_IO_CQR, (rindex (GP_CSAB_IO_CQR) & 0x88) |
		CSA_GATED_IOX0 | CSA_GATED_IOX1 | CSB_GATED_IOX0 | CSB_GATED_IOX1);

	/*
	 * Map GP_CSA to GPIO_CS0
	 * Map GP_CSB to GPIO_CS1
	 */
	windex (GP_CS_GPIO_CS_MRA, 0x10);

	/*
	 * Let CS1 and CS2 be driven by CSA and CSB now
	 */
	windex (GPIO_RBWR_A,
		rindex (GPIO_RBWR_A) & ~(GP0STAT_CTL|GP1STAT_CTL));

	/*
	 * Set PIO0 and PIO1 to Mode 0 and all Pins to Output
	 */
	outb( 0x80, PIO0_BASE + 3);
	outb( 0x80, PIO1_BASE + 3);
}


/*
 * Interrupt Handler
 */

static unsigned long counter;
static unsigned long * invocation_counter = &counter;

void phase_isr(void)
{
	rt_ack_irq(PHASE_IRQ);
	* invocation_counter = * invocation_counter + 1;
}

int init_module(void)
{
	int status;
	unsigned long startup;

	printk ("irq: init_module\n");

	mcu_setup_pio();

	/*
	 * Set DIL/NetPC INT1 (SC410 PIRQ3) to IRQ 14
	 */
	windex(ICR_B, (rindex (ICR_B) & PIRQ2S) | (PHASE_IRQ << 4));

#ifdef SHARED_MEMORY
	/*
	 * Request Shared Memory
	 */

	invocation_counter = rtai_kmalloc (0x23232323, sizeof(unsigned long));
	if(invocation_counter == NULL) {
		printk("irq: rtai_malloc failed\n");
		return EXIT_ERROR;
	}
#endif

	*invocation_counter = 0;

	/*
	 * Request IRQ from RTAI
	 */
	rt_global_cli();

	status = rt_request_global_irq(PHASE_IRQ, phase_isr);
	if(status != 0) {
		printk("irq: rt_request_global_irq failed\n");
		return EXIT_ERROR;
	}

	startup = rt_startup_irq(PHASE_IRQ);
	printk("irq: rt_startup_irq returned 0x%04lx\n", startup);

	rt_enable_irq(PHASE_IRQ);

	rt_global_sti();

	return 0;
}

void cleanup_module (void)
{
	printk ("irq: cleanup_module (%ld)\n", * invocation_counter);

	/*
	 * Disable IRQ
	 */
	rt_disable_irq(PHASE_IRQ);
	rt_free_global_irq(PHASE_IRQ);

#ifdef SHARED_MEMORY
	/*
	 * Free shared memory
	 */
	rtai_kfree((void *)invocation_counter);
#endif

	return;
}

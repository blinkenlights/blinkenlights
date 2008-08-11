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
#include <getopt.h>
#include <libgen.h>

#include "rtai_shm.h"

#define IRQ_DELAY_MAGIC    0x12345678

int
main(int argc, char *argv[])
{
	unsigned long * p_irq_delay_ns;
	char line_buffer[4096];

	/*
	 * 	
	 * 	create MCU shared memory region
	 * 	
	 */

	p_irq_delay_ns = (unsigned long *) rtai_malloc((unsigned long) IRQ_DELAY_MAGIC, sizeof(unsigned long));
	if (p_irq_delay_ns == NULL) {
		fprintf(stderr, "setirqdelay: rtai_malloc failed\n");
		exit(1);
	}


	while(gets(line_buffer) != NULL) {
		* p_irq_delay_ns = atoi(line_buffer) * 1 * 1000;  // 1 us steps
		printf("Value: %ld\n", * p_irq_delay_ns);
	}

	rtai_free(IRQ_DELAY_MAGIC, p_irq_delay_ns);
	exit(0);
}


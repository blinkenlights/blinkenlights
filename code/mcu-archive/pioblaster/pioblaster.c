#include "../windowmatrixd/bmc_pio/bmc_pio.h"

#include <stdio.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
	int pio_dev;
	char pio_mode = BMC_PIO_OUTPUT;
	unsigned char pio_data = 0xff;

	pio_dev = open("/dev/pio0", O_RDWR);

	if (pio_dev == -1) {
		perror("pioblaster: can't open PIO device");
		exit(1);
	}

	ioctl( pio_dev, BMC_PIO_CTL_DIRECTION, &pio_mode);

	write( pio_dev, &pio_data, 1);

	close( pio_dev);
}

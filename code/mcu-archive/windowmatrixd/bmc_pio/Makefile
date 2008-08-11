CFLAGS = -Wall -O2 -Wstrict-prototypes -fomit-frame-pointer
KINC   = -I/usr/src/linux/include
CC     = gcc

all: bmc_pio.o
clean:
	rm -f *.o

bmc_pio.o: bmc_pio.c
	$(CC) -D__KERNEL__ -DMODULE -I. $(KINC) $(CFLAGS) -c bmc_pio.c

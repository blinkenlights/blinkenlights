
CFLAGS=-D __KERNEL__ -D MODULE -O2 -Wall -I/home/tim/linux/include -I/home/tim/rtai-24.1.9/include

all: irq.o rt_dsiblaster.o rt_rgbblaster.o

pioblaster: pioblaster.c
	cc -o pioblaster pioblaster.c

atomicrtai:
	cc -o atomicrtai -O2 -Wall -I../linux-2.4.18/include atomicrtai.c

insmod:
	sudo insmod rt_pioblaster.o

rmmod:
	sudo rmmod rt_pioblaster

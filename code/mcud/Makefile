#SYSTEM=SYSTEM_MCU
SYSTEM=SYSTEM_DECISION

DEBUG=-g

CFLAGS=-I/usr/src/rtai-24.1.9/include -Wall -O2 -D$(SYSTEM) $(DEBUG)
MOD_CFLAGS=-D__KERNEL__ -DMODULE $(CFLAGS) # -I/usr/src/linux/include

BLIB_CFLAGS=`pkg-config --cflags blib`

MCUD_CFLAGS=-DRTAI_SHARED_MEMORY $(BLIB_CFLAGS) $(CFLAGS) # -DLONG_OPTIONS


EXPAT_CFLAGS=-I/sw/include
EXPAT_LDFLAGS=-L/sw/lib -lexpat

TARGETS=mcud mcu_bit.o mcu_led.o mcu_dsi.o mcu_rgb.o mcu_dim.o mcu_pdim.o mcu_ldim.o setports # mcu_dimtest.o irqtest.o setirqdelay


all: $(TARGETS)

clean:
	rm -f $(TARGETS)
	rm -f *.o


config.o: config.c
	$(CC) $(CFLAGS) $(BLIB_CFLAGS) $(EXPAT_CFLAGS) -c -o $@ $^

mcud.o: mcud.c
	$(CC) $(MCUD_CFLAGS) -c -o $@ $^

mcud: mcud.o config.o
	$(CC) $(CFLAGS) $(EXPAT_LDFLAGS) -o $@ $^


setirqdelay: setirqdelay.c
	$(CC) $(CFLAGS) -o $@ $^

setports.o: setports.c
	$(CC) $(CFLAGS) -c -o $@ $^

setports: setports.o
	$(CC) $(CFLAGS) -o $@ $^

mcu.o: mcu.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

pio.o: pio.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^


mcudev_bit.o: mcudev_bit.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_bit.o: mcu.o pio.o mcudev_bit.o
	$(LD) -r $^ -o $@


mcudev_led.o: mcudev_led.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_led.o: mcu.o pio.o mcudev_led.o
	$(LD) -r $^ -o $@


mcudev_pdim.o: mcudev_pdim.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_pdim.o: mcu.o pio.o mcudev_pdim.o
	$(LD) -r $^ -o $@


mcudev_ldim.o: mcudev_ldim.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_ldim.o: mcu.o pio.o mcudev_ldim.o
	$(LD) -r $^ -o $@


mcudev_dim.o: mcudev_dim.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_dim.o: mcu.o pio.o mcudev_dim.o
	$(LD) -r $^ -o $@


dimtest.o: dimtest.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_dimtest.o: mcu.o pio.o dimtest.o
	$(LD) -r $^ -o $@


irqtest.o: irqtest.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_irqtest.o: mcu.o pio.o irqtest.o
	$(LD) -r $^ -o $@


mcudev_dsi.o: mcudev_dsi.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_dsi.o: mcu.o pio.o mcudev_dsi.o
	$(LD) -r $^ -o $@


mcudev_rgb.o: mcudev_rgb.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_rgb.o: mcu.o pio.o mcudev_rgb.o
	$(LD) -r $^ -o $@



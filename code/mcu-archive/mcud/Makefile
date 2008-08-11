CFLAGS=-I /home/tim/rtai-24.1.9/include -Wall -I/sw/include
MOD_CFLAGS=-D__KERNEL__ -DMODULE -O2 -Wall -I/home/tim/linux/include -I/home/tim/rtai-24.1.9/include

MCUD_CFLAGS=#-DRTAI_SHARED_MEMORY # -DLONG_OPTIONS


EXPAT_CFLAGS=-I/sw/include
EXPAT_LDFLAGS=-L/sw/lib -lexpat

APPS=mcud sendframes
RTAPPS=setirqdelay
DRIVERS=mcu_bit.o mcu_led.o mcu_dsi.o mcu_rgb.o mcu_dim.o mcu_pdim.o irqtest.o mcu_dimtest.o
TARGETS=$(APPS) # $(RTAPPS) $(DRIVERS)


all: $(TARGETS)

clean:
	rm -f $(TARGETS)
	rm -f *.o


config.o: config.c
	$(CC) $(CFLAGS) $(EXPAT_CFLAGS) -c -o $@ $^

mcud.o: mcud.c
	$(CC) $(MCUD_CFLAGS) -c -o $@ $^

mcud: mcud.o config.o
	$(CC) $(CFLAGS) $(EXPAT_LDFLAGS) -o $@ $^


setirqdelay: setirqdelay.c
	$(CC) $(CFLAGS) -o $@ $^


mcu.o: mcu.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^


mcudev_bit.o: mcudev_bit.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_bit.o: mcu.o mcudev_bit.o
	$(LD) -r $^ -o $@


mcudev_led.o: mcudev_led.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_led.o: mcu.o mcudev_led.o
	$(LD) -r $^ -o $@


mcudev_pdim.o: mcudev_pdim.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_pdim.o: mcu.o mcudev_pdim.o
	$(LD) -r $^ -o $@


mcudev_dim.o: mcudev_dim.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_dim.o: mcu.o mcudev_dim.o
	$(LD) -r $^ -o $@


dimtest.o: dimtest.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_dimtest.o: mcu.o dimtest.o
	$(LD) -r $^ -o $@


irqtest.o: irqtest.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_irqtest.o: mcu.o irqtest.o
	$(LD) -r $^ -o $@


mcudev_dsi.o: mcudev_dsi.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_dsi.o: mcu.o mcudev_dsi.o
	$(LD) -r $^ -o $@


mcudev_rgb.o: mcudev_rgb.c
	$(CC) $(MOD_CFLAGS) -c -o $@ $^

mcu_rgb.o: mcu.o mcudev_rgb.o
	$(LD) -r $^ -o $@



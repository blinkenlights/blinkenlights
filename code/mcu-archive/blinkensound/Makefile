CFLAGS = -O2 -Wall -g 
CC = gcc
LIBS = -lm

GFX_CFLAGS = $(CFLAGS) `pkg-config --cflags directfb`
LIBS += `pkg-config --libs directfb`

default: blinkensound

audio.o: audio.c audio.h
	$(CC) $(CFLAGS) -c audio.c

midi.o: midi.c midi.h
	$(CC) $(CFLAGS) -c midi.c

disp.o: disp.c disp.h
	$(CC) $(CFLAGS) -c disp.c

gfx.o: gfx.c gfx.h
	$(CC) $(GFX_CFLAGS) -c gfx.c

fft.o: fft.c fft.h
	$(CC) $(CFLAGS) -c fft.c

net.o: net.c net.h
	$(CC) $(CFLAGS) -c net.c

usbio24.o: usbio24.c usbio24.h
	$(CC) $(CFLAGS) -c usbio24.c

voodoo.o: voodoo.c voodoo.h
	$(CC) $(CFLAGS) -c voodoo.c

main.o: main.c

blinkensound: audio.o disp.o fft.o gfx.o main.o midi.o net.o usbio24.o voodoo.o
	$(CC) -o $@ $^ $(LIBS)

clean::
	rm -f *.o *~ *.core blinkensound

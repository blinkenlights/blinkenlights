# Makefile

CFLAGS=-O2 -Wall

SOURCES=\
	hdl.h		\
	hdl-small.h	\
	hdl-medium.h	\
	blm2gif.c	\
	gif.c		\
	gif.h

all: blm2gif

blm2gif: $(SOURCES)
	$(CC) $(CFLAGS) -o blm2gif blm2gif.c gif.c

clean: 
	rm blm2gif

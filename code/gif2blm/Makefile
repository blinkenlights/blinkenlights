# Makefile

CFLAGS=-O2 -Wall

SOURCES=\
	gif2blm.c	\
	gif.c		\
	gif.h

all: gif2blm

gif2blm: $(SOURCES)
	$(CC) $(CFLAGS) -o gif2blm gif2blm.c gif.c

clean: 
	rm gif2blm

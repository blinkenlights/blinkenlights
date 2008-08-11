## Process this file with automake to produce Makefile.in

CFLAGS = -Wall -Wstrict-prototypes -O2 -g

bin_PROGRAMS = gondwana

INCLUDES = @DIRECTFB_CFLAGS@

gondwana_SOURCES = \
	effects.c	\
	effects.h	\
	gfx.c		\
	gfx.h		\
	network.c	\
	network.h	\
	magic-values.h	\
	main.c

gondwana_LDADD = @DIRECTFB_LIBS@


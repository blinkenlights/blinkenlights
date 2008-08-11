## Makefile.am for blipaq/src

bin_PROGRAMS = blipaq

AM_CPPFLAGS = \
	-DDATADIR=\"@DATADIR@/@PACKAGE@/\"	\
	-DVERSION=\"@VERSION@\"			\
	-DDEFAULT_ROTATION=@ROTATION@

INCLUDES = @DIRECTFB_CFLAGS@

blipaq_LDADD = @DIRECTFB_LIBS@

blipaq_SOURCES = \
	blipaqtypes.h	\
	display.c	\
	display.h	\
	main.c		\
	movie.c		\
	movie.h		\
	progress.c	\
	progress.h


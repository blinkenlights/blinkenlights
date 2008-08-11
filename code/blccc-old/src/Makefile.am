## Makefile.am for blccc/src

if WITH_GUI
bin_PROGRAMS = blccc
endif

INCLUDES = $(GTK_CFLAGS)
LDADD = $(GTK_LIBS)

blccc_SOURCES = \
	blccc.h 	\
	blisdn.c 	\
	blisdn.h 	\
	blmovie.c 	\
	blmovie.h 	\
	blmovielist.c 	\
	blmovielist.h 	\
	blmovieview.c 	\
	blmovieview.h 	\
	blplaylist.c 	\
	blplaylist.h 	\
	blpong.c	\
	blpong.h	\
	blpreview.c 	\
	blpreview.h 	\
	bltheater.c 	\
	bltheater.h 	\
	bltypes.h 	\
	main.c		\
	main-window.c	\
	main-window.h

EXTRA_DIST = $(blccc_SOURCES)

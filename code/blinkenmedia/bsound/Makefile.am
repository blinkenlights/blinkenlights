## Makefile.am for blinkenmedia/src

INCLUDES = @BLIB_CFLAGS@

bin_PROGRAMS = \
	bsound		

bsound_SOURCES = \
		bsound.c \
		fft.c	 \
		audio.c	 \
		midi.c	 \
		voodoo.c
bsound_LDADD = @BLIB_LIBS@ @LIBESD@ -lm

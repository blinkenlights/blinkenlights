
libdir= $(BLIB_MODULEPATH)

AM_CPPFLAGS = -DG_DISABLE_DEPRECATED

INCLUDES = @BLIB_CFLAGS@ @JNI_CFLAGS@ @DEFS@
LIBS = @BLIB_LIBS@ @JNI_LIBS@ 

lib_LTLIBRARIES = libblinkenjava.la  # libbj.la

libblinkenjava_la_SOURCES = blinkenjava.c \
	de_blinkenlights_blinkenlet_Blinkenlet.c \
	de_blinkenlights_blinkenlet_BlinkenKeyEvent.c \
	de_blinkenlights_blinkenlet_BlinkenPlayerEvent.c \
	java_lang_Throwable.c 

libblinkenjava_la_LDFLAGS = -avoid-version -module @LIBS@

#libbj_la_SOURCES = bj.c 
#libbj_la_LDFLAGS = -avoid-version -module @LIBS@

libblinkenjava_la_LDFLAGS = -avoid-version -module @LIBS@
noinst_PROGRAMS = vmtest
vmtest_SOURCES = vmtest.c

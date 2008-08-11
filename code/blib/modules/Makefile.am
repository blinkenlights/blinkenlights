## Makefile.am for blib/modules
## Process this file with automake to produce Makefile.in

libdir = @MODULEPATH@

AM_CPPFLAGS = \
	$(CFLAGS)		\
	 -DG_DISABLE_DEPRECATED

INCLUDES = \
	-I$(top_srcdir)		\
	@GLIB_CFLAGS@

if BUILD_MODULES
lib_LTLIBRARIES = \
	libbbreakout.la		\
	libbclock.la		\
	libbcountdown.la	\
	libbcreatefile.la	\
	libbdebug.la		\
	libbdropout.la		\
	libbfire.la		\
	libbfireworks.la	\
	libbmatrix.la		\
	libbpacman.la		\
	libbpong.la		\
	libbpongmulti.la	\
	libbproxy.la		\
	libbpushline.la 	\
	libbqix.la		\
	libbretris.la	 	\
	libbsitris4.la	 	\
	libbsnake.la		\
	libbtetris.la		\
	libbtext.la		\
	libbxxo.la
endif

libbbreakout_la_SOURCES = bbreakout.c
libbbreakout_la_LDFLAGS = -avoid-version -module

libbclock_la_SOURCES = bclock.c digits.h
libbclock_la_LDFLAGS = -avoid-version -module

libbcountdown_la_SOURCES = bcountdown.c digits.h
libbcountdown_la_LDFLAGS = -avoid-version -module

libbcreatefile_la_SOURCES = bcreatefile.c
libbcreatefile_la_LDFLAGS = -avoid-version -module

libbdebug_la_SOURCES = bdebug.c digits.h
libbdebug_la_LDFLAGS = -avoid-version -module

libbdropout_la_SOURCES = bdropout.c
libbdropout_la_LDFLAGS = -avoid-version -module

libbfire_la_SOURCES = bfire.c
libbfire_la_LDFLAGS = -avoid-version -module

libbfireworks_la_SOURCES = bfireworks.c
libbfireworks_la_LDFLAGS = -avoid-version -module

libbmatrix_la_SOURCES = bmatrix.c
libbmatrix_la_LDFLAGS = -avoid-version -module

libbpacman_la_SOURCES = bpacman.c
libbpacman_la_LDFLAGS = -avoid-version -module

libbpong_la_SOURCES = bpong.c digits.h
libbpong_la_LDFLAGS = -avoid-version -module

libbpongmulti_la_SOURCES = bpongmulti.c digits.h
libbpongmulti_la_LDFLAGS = -avoid-version -module

libbproxy_la_SOURCES = bproxy.c
libbproxy_la_LDFLAGS = -avoid-version -module

libbpushline_la_SOURCES = bpushline.c
libbpushline_la_LDFLAGS = -avoid-version -module

libbqix_la_SOURCES = bqix.c
libbqix_la_LDFLAGS = -avoid-version -module

libbretris_la_SOURCES = bretris.c
libbretris_la_LDFLAGS = -avoid-version -module

libbsitris4_la_SOURCES = bsitris4.c
libbsitris4_la_LDFLAGS = -avoid-version -module

libbsnake_la_SOURCES = bsnake.c
libbsnake_la_LDFLAGS = -avoid-version -module

libbtetris_la_SOURCES = btetris.c
libbtetris_la_LDFLAGS = -avoid-version -module

libbtext_la_SOURCES = btext.c characters.c characters.h
libbtext_la_LDFLAGS = -avoid-version -module

libbxxo_la_SOURCES = bxxo.c
libbxxo_la_LDFLAGS = -avoid-version -module

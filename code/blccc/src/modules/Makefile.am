## Makefile.am for blccc/src/modules

libdir = $(prefix)/lib/blccc/modules

AM_CPPFLAGS = -DG_DISABLE_DEPRECATED

INCLUDES = @GLIB_CFLAGS@ @BLIB_CFLAGS@

lib_LTLIBRARIES = libtest.la

libtest_la_SOURCES = test.c test.h
libtest_la_LDFLAGS = -avoid-version -module
libtest_la_LIBADD  = @GLIB_LIBS@ @BLIB_LIBS@

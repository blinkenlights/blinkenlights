## Makefile.am for blib/test/modules

AM_CPPFLAGS = -DG_DISABLE_DEPRECATED

INCLUDES = -I$(top_srcdir) @GLIB_CFLAGS@

noinst_LTLIBRARIES = libtest.la

libtest_la_SOURCES = test.c
libtest_la_LDFLAGS = -avoid-version -module

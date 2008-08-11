
## Makefile.am for blib/test
## Process this file with automake to produce Makefile.in

SUBDIRS = movies modules

INCLUDES = -I$(top_srcdir)

AM_CPPFLAGS = $(GLIB_CFLAGS) @GTK_CFLAGS@

if HAVE_GTK
gtk_test_programs=test-module
endif

noinst_PROGRAMS = \
	test-movie-parser	\
	test-movie-player	\
	test-movie-writer	\
	test-theme-parser	\
	test-sender		\
	test-receiver		\
	test-module-scanner	\
	$(gtk_test_programs)

BLIB_LIBS = @GLIB_LIBS@ @WIN32_LIBS@ ../blib/libb.la

test_movie_parser_SOURCES = test-movie-parser.c
test_movie_parser_LDADD   = $(BLIB_LIBS)

test_movie_player_SOURCES = test-movie-player.c
test_movie_player_LDADD   = $(BLIB_LIBS)

test_movie_writer_SOURCES = test-movie-writer.c
test_movie_writer_LDADD   = $(BLIB_LIBS)

test_theme_parser_SOURCES = test-theme-parser.c
test_theme_parser_LDADD   = $(BLIB_LIBS)

test_sender_SOURCES = test-sender.c
test_sender_LDADD   = $(BLIB_LIBS)

test_receiver_SOURCES = test-receiver.c
test_receiver_LDADD   = $(BLIB_LIBS)

test_module_scanner_SOURCES = test-module-scanner.c
test_module_scanner_LDADD = $(BLIB_LIBS)

if HAVE_GTK
test_module_SOURCES = test-module.c
test_module_LDADD = @GTK_LIBS@ $(BLIB_LIBS)
endif

TESTS = run-tests.sh

EXTRA_DIST = $(TESTS)

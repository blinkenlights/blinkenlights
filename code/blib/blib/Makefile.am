## Makefile.am for blib/blib
## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	$(CFLAGS)			\
	 -DG_DISABLE_DEPRECATED		\
	-DTHEMEPATH=\"$(THEMEPATH)\"	\
	-DMODULEPATH=\"$(MODULEPATH)\"

INCLUDES = \
	-I$(top_srcdir)	\
	@GLIB_CFLAGS@

lib_LTLIBRARIES = libb.la

libb_la_includedir = $(INCLUDEDIR)/blib

libb_gif_public_headers = \
	gif-load.h	\
	gif-save.h	\
	gif-types.h

libb_gif_sources = \
	gif-load.c	\
	gif-save.c

libb_la_include_HEADERS = \
	blib.h			\
	btypes.h		\
	beffects.h		\
	bmovie.h		\
	bmovie-blm.h		\
	bmovie-bml.h		\
	bmovie-gif.h		\
	bmovie-effects.h	\
	bobject.h		\
	bparams.h		\
	bpacket.h		\
	bparser.h		\
	btheme.h		\
	bthemes.h		\
	bproxyclient.h		\
	bproxyserver.h		\
	breceiver.h		\
	bsender.h		\
	bsocket.h		\
	butils.h		\
	bwriter.h		\
	bprotocol.h		\
	bmodule.h		\
	bmodule-internal.h	\
	bmodule-utils.h		\
	bmoduleinfo.h		\
	bmovieplayer.h		\
	$(libb_gif_public_headers)

libb_la_sources = \
	blib.c			\
	beffects.c		\
	bmodule.c		\
	bmodule-utils.c		\
	bmoduleinfo.c		\
	bmovieplayer.c		\
	bmovie.c		\
	bmovie-blm.c		\
	bmovie-bml.c		\
	bmovie-bml-parser.c	\
	bmovie-bml-parser.h	\
	bmovie-gif.c		\
	bmovie-gif-parser.c	\
	bmovie-gif-parser.h	\
	bmovie-effects.c	\
	bobject.c		\
	bpacket.c		\
	bparams.c		\
	bparser.c		\
	bproxyclient.c		\
	bproxyserver.c		\
	breceiver.c		\
	bsender.c		\
	bsocket.c		\
	btheme.c		\
	bthemes.c		\
	btheme-parser.c		\
	btheme-parser.h		\
	btypes.c		\
	butils.c		\
	bwriter.c		\
	$(libb_gif_sources)

libb_la_built_sources = \
	bmarshal.c	\
	bmarshal.h

libb_la_extra_sources = \
	bmarshal.list

libb_la_SOURCES = $(libb_la_built_sources) $(libb_la_sources)


libb_la_LDFLAGS = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE)	\
	-release $(LT_RELEASE) -export-dynamic

libb_la_LIBADD = @GLIB_LIBS@ @WIN32_LIBS@


EXTRA_DIST = $(libb_la_extra_sources)


#
# rules to generate built sources
#
# setup autogeneration dependencies
gen_sources = xgen-bmh xgen-bmc
CLEANFILES = $(gen_sources)

$(srcdir)/bmarshal.h: $(srcdir)/bmarshal.list
	$(GLIB_GENMARSHAL) --prefix=b_marshal $(srcdir)/bmarshal.list --header >> xgen-bmh \
	&& (cmp -s xgen-bmh $(@F) || cp xgen-bmh $(@F)) \
	&& rm -f xgen-bmh xgen-bmh~

$(srcdir)/bmarshal.c: $(srcdir)/bmarshal.h
	echo "#include \"bmarshal.h\"" >> xgen-bmc \
	&& $(GLIB_GENMARSHAL) --prefix=b_marshal $(srcdir)/bmarshal.list --body >> xgen-bmc \
	&& cp xgen-bmc $(@F) \
	&& rm -f xgen-bmc xgen-bmc~

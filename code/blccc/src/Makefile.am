## Makefile.am for blccc/src

SUBDIRS = modules

bin_PROGRAMS = blccc

AM_CPPFLAGS = -DG_DISABLE_DEPRECATED

INCLUDES = @GLIB_CFLAGS@ @BLIB_CFLAGS@
LDADD    = @GLIB_LIBS@ @BLIB_LIBS@ -lm

CLEANFILES = 

blccc_sources = \
	blaccept.c 		\
	blaccept.h 		\
	blapp.c 		\
	blapp.h 		\
	blccc.c 		\
	blccc.h 		\
	blconfig.c 		\
	blconfig.h 		\
	bldispatch.c 		\
	bldispatch.h 		\
	blisdn.c 		\
	blisdn.h 		\
	bllisten.c 		\
	bllisten.h 		\
	bllogger.c 		\
	bllogger.h 		\
	blloveletters.c		\
	blloveletters.h		\
	bllovemodules.c		\
	bllovemodules.h		\
	blmarshal.list		\
	blondemand.c		\
	blondemand.h		\
	blplaylist.c		\
	blplaylist.h		\
	blplaylist-parse.c	\
	blplaylist-parse.h	\
	blplaylistitem.c	\
	blplaylistitem.h	\
	bltheater.c 		\
	bltheater.h 		\
	bltypes.h 		\
	main.c

blccc_built_sources = \
	blmarshal.c		\
	blmarshal.h

blccc_SOURCES  = $(blccc_built_sources) $(blccc_sources)

gen_sources = xgen-gmh xgen-gmc
CLEANFILES += $(gen_sources)

$(srcdir)/blmarshal.h: blmarshal.list
	$(GLIB_GENMARSHAL) --prefix=bl_marshal $(srcdir)/blmarshal.list --header >> xgen-gmh \
	&& (cmp -s xgen-gmh $@ || cp xgen-gmh $@) \
	&& rm -f xgen-gmh xgen-gmh~
$(srcdir)/blmarshal.c: $(srcdir)/blmarshal.h
	echo "#include \"blmarshal.h\"" >> xgen-gmc \
	&& $(GLIB_GENMARSHAL) --prefix=bl_marshal $(srcdir)/blmarshal.list --body >> xgen-gmc \
	&& cp xgen-gmc $@ \
	&& rm -f xgen-gmc xgen-gmc~

## Process this file with automake to produce Makefile.in

if HAVE_DIRECTFB
directfb_programs = blinkensim-directfb
endif

if HAVE_GTK
gtk_programs = blinkensim-gtk
endif

if HAVE_AA
aa_programs = blinkensim-aa
endif

bin_PROGRAMS = $(directfb_programs) $(gtk_programs) $(aa_programs)

INCLUDES = @DIRECTFB_CFLAGS@ @GTK_CFLAGS@ @AA_CFLAGS@

blinkensim_directfb_SOURCES = \
	gfx-directfb.c	\
	gfx.h		\
	main.c

blinkensim_directfb_LDADD = @DIRECTFB_LIBS@

blinkensim_gtk_SOURCES = \
	gfx-gtk.c	\
	gfx.h		\
	main.c

blinkensim_gtk_LDADD = @GTK_LIBS@

blinkensim_aa_SOURCES = \
	gfx-aa.c	\
	gfx.h		\
	main.c

blinkensim_aa_LDADD = @AA_LIBS@

install-exec-hook:
if DEFAULT_BINARY
	cd $(DESTDIR)$(bindir) \
	&& rm -f blinkensim$(EXEEXT) \
	&& $(LN_S) $(default_binary)$(EXEEXT) blinkensim$(EXEEXT)
endif

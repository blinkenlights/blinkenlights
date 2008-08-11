## Makefile.am for blinkentools/src

INCLUDES = \
	@BLIB_CFLAGS@	\
	@PIXBUF_CFLAGS@	\
	@AA_CFLAGS@

if HAVE_AA
aa_programs = bplay
endif

if HAVE_LIBMNG
mng_programs = b2mng
endif

if HAVE_PIXBUF
pixbuf_programs = b2image
endif

if ENABLE_OLDTOOLS
oldtools = blm2gif blmplay gif2blm
endif

bin_PROGRAMS = \
	b2b			\
	bsend			\
	bmultiplexer		\
	blinkenproxy		\
	$(aa_programs)		\
	$(mng_programs)		\
	$(pixbuf_programs)	\
	$(oldtools)

blutils_sources = blutils.h blutils.c

blimage_sources = \
	hdl.h		\
	hdl-small.h	\
	hdl-medium.h	\
	hdl-large.h	\
	hdl-huge.h

blm2gif_SOURCES = \
	blm2gif.c	\
	gif-save.c	\
	gif-save.h	\
	$(blimage_sources) \
	$(blutils_sources)

blmplay_SOURCES = \
	blmplay.c	\
	$(blutils_sources)

gif2blm_SOURCES = \
	gif2blm.c	\
	gif-load.c	\
	gif-load.h	\
	$(blimage_sources)

bmultiplexer_SOURCES = \
	bmultiplexer.c
bmultiplexer_LDADD = @BLIB_LIBS@

b2b_SOURCES = b2b.c
b2b_LDADD = @BLIB_LIBS@ -lm

b2image_SOURCES = b2image.c
b2image_LDADD = @PIXBUF_LIBS@

b2mng_SOURCES = b2mng.c	bmovie-mng.c bmovie-mng.h
b2mng_LDADD = @LIBMNG@ @BLIB_LIBS@

bsend_SOURCES = bsend.c
bsend_LDADD = @BLIB_LIBS@

bplay_SOURCES = bplay.c
bplay_LDADD = @AA_LIBS@

blinkenproxy_SOURCES = blinkenproxy.c
blinkenproxy_LDADD = @BLIB_LIBS@


install-exec-hook:
	cd $(DESTDIR)$(bindir) \
	&& rm -f b2blm b2bml b2gif \
	&& $(LN_S) b2b b2blm && $(LN_S) b2b b2bml && $(LN_S) b2b b2gif
if HAVE_PIXBUF
	cd $(DESTDIR)$(bindir) \
	&& rm -f b2jpeg b2png \
	&& $(LN_S) b2image b2jpeg && $(LN_S) b2image b2png
endif

uninstall-local:
	rm -f $(DESTDIR)$(bindir)/b2blm \
	      $(DESTDIR)$(bindir)/b2bml \
	      $(DESTDIR)$(bindir)/b2gif
if HAVE_PIXBUF
	rm -f $(DESTDIR)$(bindir)/b2jpeg\
	      $(DESTDIR)$(bindir)/b2png
endif

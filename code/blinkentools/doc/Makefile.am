## Makefile.am for doc

if HAVE_AA
bplay = bplay.1
endif

if HAVE_LIBMNG
b2mng = b2mng.1
endif

if HAVE_PIXBUF
b2image = b2image.1
endif

man_MANS = \
	b2b.1		\
	bsend.1		\
	bmultiplexer.1	\
	blinkenproxy.1	\
	$(b2image)	\
	$(b2mng)	\
	$(bplay)


install-data-hook:
	cd $(DESTDIR)$(mandir)/man1; \
	for i in b2blm b2bml b2gif; do \
	  rm -f $$i.1; \
	  $(LN_S) b2b.1 $$i.1; \
	done
if HAVE_PIXBUF
	cd $(DESTDIR)$(mandir)/man1; \
	for i in b2jpeg b2png; do \
	  rm -f $$i.1; \
	  $(LN_S) b2image.1 $$i.1; \
	done
endif

uninstall-local:
	for i in b2blm b2bml b2gif; do \
	  rm -f $(DESTDIR)$(mandir)/man1/$$i.1; \
	done
if HAVE_PIXBUF
	for i in b2jpeg b2png; do \
	  rm -f $(DESTDIR)$(mandir)/man1/$$i.1; \
	done
endif

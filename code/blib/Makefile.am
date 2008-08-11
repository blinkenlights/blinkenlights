## Makefile.am for blib
## Process this file with automake to produce Makefile.in

SUBDIRS = blib gfx modules test data docs

if HAVE_AA
AA_PC = blib-aa.pc
endif

if HAVE_DIRECTFB
DIRECTFB_PC = blib-directfb.pc
endif

if HAVE_GTK
GTK_PC = blib-gtk.pc
endif

if HAVE_PIXBUF
PIXBUF_PC = blib-pixbuf.pc
endif

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = \
	blib.pc		\
	$(AA_PC)	\
	$(DIRECTFB_PC)	\
	$(GTK_PC)	\
	$(PIXBUF_PC)

EXTRA_DIST = \
	autogen.sh			\
	debian/changelog		\
	debian/control			\
	debian/copyright		\
	debian/dirs			\
	debian/docs			\
	debian/files			\
	debian/libblib-dev.files	\
	debian/libblib-directfb-dev.files\
	debian/libblib-directfb.files	\
	debian/libblib-gtk-dev.files	\
	debian/libblib-gtk.files	\
	debian/libblib.files		\
	debian/rules

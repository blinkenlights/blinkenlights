## Makefile.am for blib/gfx
## Process this file with automake to produce Makefile.in

AM_CPPFLAGS = \
	$(CFLAGS)			\
	-DG_DISABLE_DEPRECATED		\
	-DGDK_PIXBUF_DISABLE_DEPRECATED	\
	-DGDK_DISABLE_DEPRECATED	\
	-DGTK_DISABLE_DEPRECATED

INCLUDES = \
	-I$(top_srcdir)		\
	@GLIB_CFLAGS@		\
	@GDK_PIXBUF_CFLAGS@ 	\
	@GTK_CFLAGS@ 		\
	@DIRECTFB_CFLAGS@

gfx_ldflags = \
	-version-info $(LT_CURRENT):$(LT_REVISION):$(LT_AGE) \
	-release $(LT_RELEASE) -export-dynamic


if HAVE_AA
gfx_aa_lib = libb-aa.la
endif

if HAVE_DIRECTFB
gfx_directfb_lib = libb-directfb.la
endif

if HAVE_GTK
gfx_gtk_lib = libb-gtk.la
endif

if HAVE_PIXBUF
gfx_pixbuf_lib = libb-pixbuf.la
endif


lib_LTLIBRARIES = \
	$(gfx_directfb_lib)	\
	$(gfx_pixbuf_lib)	\
	$(gfx_gtk_lib)		\
	$(gfx_aa_lib)

libb_aa_la_includedir      = $(INCLUDEDIR)/blib
libb_aa_la_include_HEADERS = blib-aa.h bview-aa.h
libb_aa_la_SOURCES         = bview-aa.c
libb_aa_la_LIBADD          = @AA_LIBS@ @WIN32_LIBS@
libb_aa_la_LDFLAGS         = $(gfx_ldflags)

libb_directfb_la_includedir      = $(INCLUDEDIR)/blib
libb_directfb_la_include_HEADERS = blib-directfb.h bview-directfb.h
libb_directfb_la_SOURCES         = bview-directfb.c
libb_directfb_la_LIBADD          = @DIRECTFB_LIBS@ @WIN32_LIBS@
libb_directfb_la_LDFLAGS         = $(gfx_ldflags)

libb_gtk_la_includedir      = $(INCLUDEDIR)/blib
libb_gtk_la_include_HEADERS = blib-gtk.h bview-gtk.h
libb_gtk_la_SOURCES         = bview-gtk.c
libb_gtk_la_LIBADD          = @GTK_LIBS@ @WIN32_LIBS@
libb_gtk_la_LDFLAGS         = $(gfx_ldflags)

libb_pixbuf_la_includedir      = $(INCLUDEDIR)/blib
libb_pixbuf_la_include_HEADERS = blib-pixbuf.h bview-pixbuf.h
libb_pixbuf_la_SOURCES         = bview-pixbuf.c
libb_pixbuf_la_LIBADD          = @GDK_PIXBUF_LIBS@ @WIN32_LIBS@
libb_pixbuf_la_LDFLAGS         = $(gfx_ldflags)

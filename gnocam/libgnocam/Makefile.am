INCLUDES =						\
	-I$(top_srcdir) -I$(top_builddir)		\
	$(LIBGNOCAM_CFLAGS)				\
	-DIMAGEDIR=\"$(prefix)/share/pixmaps\"		\
	-DGNOCAM_GLADE_DIR=\"$(GNOCAM_GLADE_DIR)\"	\
	-DGNOCAM_SRCDIR=\"$(top_srcdir)\"

CORBA_GENERATED = GNOME_C-common.c GNOME_C-stubs.c GNOME_C.h
$(CORBA_GENERATED): $(top_srcdir)/idl/GNOME_C.idl
	$(ORBIT_IDL) --noskels \
		-I`$(PKG_CONFIG) --variable=idldir libbonobo-2.0` \
		-I`$(PKG_CONFIG) --variable=idldir bonobo-activation-2.0` \
		$(top_srcdir)/idl/GNOME_C.idl

noinst_LTLIBRARIES = libgnocam.la
libgnocam_la_SOURCES = 				\
	$(CORBA_GENERATED) 			\
	gnocam-util.c gnocam-util.h

libgnocam_la_LDFLAGS = $(LIBGNOCAM_LIBS)

Gladedir = $(GNOCAM_GLADE_DIR)
Glade_DATA = gnocam.glade

CLEANFILES =$(CORBA_GENERATED)

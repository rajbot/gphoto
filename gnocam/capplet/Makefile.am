SUBDIRS = help

INCLUDES = 					\
 -I$(top_srcdir)				\
 -DIMAGEDIR=\"$(datadir)/pixmaps/$(PACKAGE)/\"	\
 -DGNOME_LOCALEDIR=\""$(datadir)/locale"\"	\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 $(GNOCAM_CAPPLET_CFLAGS)

CORBA_GENERATED = GNOME_C-common.c GNOME_C-stubs.c GNOME_C.h
$(CORBA_GENERATED): $(top_srcdir)/idl/GNOME_C.idl
	$(ORBIT_IDL) --noskels \
		-I`$(PKG_CONFIG) --variable=idldir libbonobo-2.0` \
		-I`$(PKG_CONFIG) --variable=idldir bonobo-activation-2.0` \
		$(top_srcdir)/idl/GNOME_C.idl

bin_PROGRAMS = gnocam-capplet
gnocam_capplet_LDADD = $(top_builddir)/libgnocam/libgnocam.la	\
	$(GNOCAM_CAPPLET_LIBS)
gnocam_capplet_SOURCES =
	gnocam-capplet.c gnocam-capplet.h	\
	main.c 					\
	$(CORBA_GENERATED)			\
	gnocam-chooser.c gnocam-chooser.h

@INTLTOOL_DESKTOP_RULE@

desktopdir = $(datadir)/control-center-2.0/capplets
Desktop_in_files = gnocam-capplet.desktop.in
desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

$(Desktop_in_files): $(Desktop_in_files:.desktop.in=.desktop.in.in)
	sed -e "s|\@PREFIX\@|$(prefix)|" $< > $@

Gladedir = $(GNOCAM_GLADE_DIR)
Glade_DATA = gnocam-capplet.glade

CLEANFILES = gnocam-capplet.desktop.in gnocam-capplet.desktop $(CORBA_GENERATED)
EXTRA_DIST = $(Desktop_in_files) gnocam-capplet.desktop.in.in

INCLUDES =					\
 -I$(top_srcdir)				\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"			\
 -DIMAGEDIR=\"$(datadir)/pixmaps/$(PACKAGE)/\"	\
 -DUIDIR="\"$(datadir)/$(PACKAGE)/ui/\""	\
 -DGNOMELOCALEDIR=\""$(datadir)/locale"\"	\
 $(APPLET_CFLAGS)

libexec_PROGRAMS = gnocam-applet
gnocam_applet_LDADD = $(top_builddir)/libgnocam/libgnocam.la $(APPLET_LIBS)

CORBA_GENERATED = GNOME_C.h GNOME_C-common.c GNOME_C-stubs.c

gnocam_applet_SOURCES =					\
 $(CORBA_GENERATED)					\
 gnocam-applet.c gnocam-applet.h			\
 gnocam-applet-cam.c gnocam-applet-cam.h		\
 gnocam-applet-marshal.c gnocam-applet-marshal.h	\
 main.c

$(CORBA_GENERATED): $(top_srcdir)/idl/GNOME_C.idl
	$(ORBIT_IDL) --noskels \
		-I`$(PKG_CONFIG) --variable=idldir libbonobo-2.0` \
		-I`$(PKG_CONFIG) --variable=idldir bonobo-activation-2.0` \
		$(top_srcdir)/idl/GNOME_C.idl

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GnocamApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

EXTRA_DIST =				\
	gnocam-applet-marshal.list	\
	GNOME_GnocamApplet.server.in.in	\
	gnocam-applet.schemas

GENMARSHAL_GENERATED = gnocam-applet-marshal.c gnocam-applet-marshal.h

@INTLTOOL_SERVER_RULE@

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|; s|\@PREFIX\@|$(prefix)|;" $< > $@

uidir   = $(datadir)/$(PACKAGE)/ui
ui_DATA = GNOME_GnocamApplet.xml gnocam-viewer-ui.xml

schemadir   = @GCONF_SCHEMA_FILE_DIR@
schema_DATA = gnocam-applet.schemas

gnocam-applet-marshal.h: gnocam-applet-marshal.list
	( @GLIB_GENMARSHAL@ --prefix=gnocam_applet_marshal $(top_srcdir)/applet/gnocam-applet-marshal.list --header > gnocam-applet-marshal.tmp \
	  && mv gnocam-applet-marshal.tmp gnocam-applet-marshal.h ) \
	|| ( rm -f gnocam-applet-marshal.tmp && exit 1 )

gnocam-applet-marshal.c: gnocam-applet-marshal.h
	( echo "#include \"gnocam-applet-marshal.h\"" >gnocam-applet-marshal.tmp \
	  && @GLIB_GENMARSHAL@ --prefix=gnocam_applet_marshal $(top_srcdir)/applet/gnocam-applet-marshal.list --body >> gnocam-applet-marshal.tmp \
	  && mv gnocam-applet-marshal.tmp gnocam-applet-marshal.c) \
	|| ( rm -f gnocam-applet-marshal.tmp && exit 1 )

BUILT_SOURCES = $(CORBA_GENERATED) $(GENMARSHAL_GENERATED)	\
	GNOME_GnocamApplet.server GNOME_GnocamApplet.server.in
CLEANFILES = $(BUILT_SOURCES) $(GENMARSHAL_GENERATED)

install-data-local:
	GCONF_CONFIG_SOURCE=$(GCONF_SCHEMA_CONFIG_SOURCE) $(GCONFTOOL) --makefile-install-rule $(srcdir)/$(schema_DATA) ;



INCLUDES =\
 -I$(top_srcdir)\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 -DIMAGEDIR=\"$(prefix)/share/pixmaps\"\
 -DGNOMELOCALEDIR=\""$(datadir)/locale"\"\
 $(APPLET_CFLAGS)

libexec_PROGRAMS = gnocam-applet

CORBA_GENERATED = GnoCam.h GnoCam-common.c GnoCam-stubs.c

gnocam_applet_SOURCES =\
 $(CORBA_GENERATED)\
 gnocam-applet.c gnocam-applet.h \
 main.c

$(CORBA_GENERATED): $(top_srcdir)/idl/GnoCam.idl
	$(ORBIT_IDL) --noskels \
		-I`$(PKG_CONFIG) --variable=idldir libbonobo-2.0` \
		-I`$(PKG_CONFIG) --variable=idldir bonobo-activation-2.0` \
		$(top_srcdir)/idl/GnoCam.idl

gnocam_applet_LDADD = $(APPLET_LIBS)

serverdir       = $(libdir)/bonobo/servers
server_in_files = GNOME_GnoCamApplet.server.in
server_DATA     = $(server_in_files:.server.in=.server)

EXTRA_DIST = GNOME_GnoCamApplet.server.in.in

@INTLTOOL_SERVER_RULE@

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|; s|\@PREFIX\@|$(prefix)|;" $< > $@

BUILT_SOURCES = $(CORBA_GENERATED) \
	GNOME_GnoCamApplet.server GNOME_GnoCamApplet.server.in
CLEANFILES = $(BUILT_SOURCES)

INCLUDES =\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 -DLOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"\
 $(GNOCAM_CFLAGS)

bin_PROGRAMS=gnocam

CORBA_GENERATED = GnoCam.h GnoCam-common.c GnoCam-stubs.c GnoCam-skels.c

gnocam_SOURCES =					\
 $(CORBA_GENERATED)					\
 bonobo-storage-camera.c bonobo-storage-camera.h	\
 bonobo-stream-camera.c bonobo-stream-camera.h		\
 main.c							\
 gnocam-cache.c  gnocam-cache.h				\
 gnocam-camera.c gnocam-camera.h			\
 gnocam-main.c gnocam-main.h				\
 gnocam-util.c gnocam-util.h

gnocam_LDADD = $(GNOCAM_LIBS)

$(CORBA_GENERATED): $(top_srcdir)/idl/GnoCam.idl
	$(ORBIT_IDL) -I`$(PKG_CONFIG) --variable=idldir libbonobo-2.0` \
		-I`$(PKG_CONFIG) --variable=idldir bonobo-activation-2.0` \
		$(top_srcdir)/idl/GnoCam.idl

serverdir = $(libdir)/bonobo/servers
server_in_files = GNOME_GnoCam.server.in
server_DATA = $(server_in_files:.server.in=.server)

EXTRA_DIST = GNOME_GnoCam.server.in.in

@INTLTOOL_SERVER_RULE@

$(server_in_files): $(server_in_files:.server.in=.server.in.in)
	sed -e "s|\@PREFIX\@|$(prefix)|" $< > $@

BUILT_SOURCES = $(CORBA_GENERATED) GNOME_GnoCam.server.in GNOME_GnoCam.server
CLEANFILES    = $(CORBA_GENERATED) GNOME_GnoCam.server.in GNOME_GnoCam.server

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

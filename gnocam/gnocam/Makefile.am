INCLUDES =\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 -DBONOBO_DISABLE_DEPRECATED\
 -DLOCALEDIR=\""$(prefix)/$(DATADIRNAME)/locale"\"\
 -DGTK_DISABLE_DEPRECATED\
 $(GNOCAM_CFLAGS)

bin_PROGRAMS=gnocam

CORBA_SOURCES = GnoCam.h GnoCam-common.c GnoCam-stubs.c	GnoCam-skels.c

gnocam_SOURCES =					\
 $(CORBA_SOURCES)					\
 bonobo-storage-camera.c bonobo-storage-camera.h	\
 bonobo-stream-camera.c bonobo-stream-camera.h		\
 main.c							\
 gnocam-camera.c gnocam-camera.h			\
 gnocam-main.c gnocam-main.h				\
 gnocam-util.c gnocam-util.h

gnocam_LDADD = $(GNOCAM_LIBS)

idls = $(top_srcdir)/idl/GnoCam.idl
idl_flags =\
 -I$(BONOBO_IDLDIR) -I$(BONOBO_ACT_IDLDIR) -I$(top_srcdir)/idl\
 -D__GNOCAM_COMPILATION

$(CORBA_SOURCES): $(idls)
	$(ORBIT_IDL)   $(top_srcdir)/idl/GnoCam.idl $(idl_flags)

GnoCam.h: $(idls)

serverdir = $(libdir)/bonobo/servers
server_in_files = GNOME_GnoCam.server.in
server_DATA = $(server_in_files:.server.in=.server)

BUILT_SOURCES=$(CORBA_SOURCES)
CLEANFILES += $(BUILT_SOURCES) $(server_DATA)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

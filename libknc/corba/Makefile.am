SUBDIRS = icons

INCLUDES =						\
	-I$(top_srcdir)					\
	-DICONSDIR="\"$(datadir)/pixmaps/knc/\""	\
	$(CORBA_CFLAGS)

CORBA_GENERATED = GNOME_C-common.c GNOME_C-stubs.c GNOME_C-skels.c GNOME_C.h
$(CORBA_GENERATED): $(GNOME_C_IDL)
	$(ORBIT_IDL) \
		-I`$(PKG_CONFIG) --variable=idldir libbonobo-2.0` \
		-I`$(PKG_CONFIG) --variable=idldir bonobo-activation-2.0` \
		$(GNOME_C_IDL)

if HAVE_CORBA
libexec_PROGRAMS = knc
endif

knc_SOURCES = $(CORBA_GENERATED)	\
	knc.c				\
	knc-c-bag.h	knc-c-bag.c	\
	knc-c-camera.h	knc-c-camera.c	\
	knc-c-dir.h	knc-c-dir.c	\
	knc-c-file.h	knc-c-file.c	\
	knc-c-if.h	knc-c-if.c	\
	knc-c-prop.h	knc-c-prop.c	\
	knc-c-mngr.h	knc-c-mngr.c
knc_LDADD = ../libknc/libknc.la $(CORBA_LIBS)

GNOME_C_Knc.server.in: GNOME_C_Knc.server.in.in
	sed -e "s|\@LIBEXECDIR\@|$(libexecdir)|" $< > $@

if HAVE_CORBA
serverdir = $(libdir)/bonobo/servers
server_in_files = GNOME_C_Knc.server.in
server_DATA = $(server_in_files:.server.in=.server)
@INTLTOOL_SERVER_RULE@
endif

CLEANFILES = $(CORBA_GENERATED)

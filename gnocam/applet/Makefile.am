
INCLUDES =\
 -I$(top_srcdir)\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 -DIMAGEDIR=\"$(prefix)/share/pixmaps\"\
 -DGNOMELOCALEDIR=\""$(datadir)/locale"\"\
 $(APPLET_CFLAGS)

if APPLET
bin_PROGRAMS=gnocam-applet
else
bin_PROGRAMS=
endif

CORBA_GENERATED = GnoCam.h GnoCam-common.c GnoCam-skels.c GnoCam-stubs.c

gnocam_applet_SOURCES =\
 $(CORBA_GENERATED)\
 gnocam-applet.c

$(CORBA_GENERATED): $(top_srcdir)/idl/GnoCam.idl
	$(ORBIT_IDL) $(top_srcdir)/idl/GnoCam.idl `$(GNOME_CONFIG) --cflags idl`

BUILT_SOURCES=$(CORBA_GENERATED)
CLEANFILES=$(CORBA_GENERATED)

gnocam_applet_LDADD = $(APPLET_LIBS)

sysdir=$(datadir)/applets/Multimedia
sys_predata=gnocam-applet.desktop.in
if APPLET
sys_DATA=$(sys_predata:.desktop.in=.desktop)
endif
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir=$(sysconfdir)/CORBA/servers
if APPLET
gnorba_DATA=gnocam-applet.gnorba
endif

EXTRA_DIST=gnocam-applet.gnorba gnocam-applet.desktop

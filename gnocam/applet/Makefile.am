
INCLUDES =\
 -I$(top_srcdir)\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 -DIMAGEDIR=\"$(prefix)/share/pixmaps\"\
 -DGNOMELOCALEDIR=\""$(datadir)/locale"\"\
 $(APPLET_CFLAGS)

bin_PROGRAMS = gnocam-applet

CORBA_GENERATED =\
 GnoCam.h\
 GnoCam-common.c\
 GnoCam-skels.c\
 GnoCam-stubs.c

gnocam_applet_SOURCES =\
 $(CORBA_GENERATED)\
 gnocam-applet.c

$(CORBA_GENERATED): $(top_srcdir)/idl/GnoCam.idl
	$(ORBIT_IDL) $(top_srcdir)/idl/GnoCam.idl `$(GNOME_CONFIG) --cflags idl`

BUILT_SOURCES += $(CORBA_GENERATED)
CLEANFILES += $(CORBA_GENERATED)

gnocam_applet_LDADD = $(APPLET_LIBS)

sysdir = $(datadir)/applets/Multimedia
sys_predata = gnocam-applet.desktop.in
sys_DATA = $(sys_predata:.desktop.in=.desktop)
@XML_I18N_MERGE_DESKTOP_RULE@

gnorbadir = $(sysconfdir)/CORBA/servers
gnorba_DATA = gnocam-applet.gnorba

EXTRA_DIST = $(gnorba_DATA) $(sys_DATA)

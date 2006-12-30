INCLUDES =\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 $(EXTRA_GNOME_CFLAGS)\
 $(GPHOTO_CFLAGS)

oafdir=$(datadir)/oaf
oaf_DATA=Bonobo_Moniker_camera.oaf

Bonobo_Moniker_camera.oaf: $(srcdir)/Bonobo_Moniker_camera.oaf.in $(top_builddir)/config.status
	sed -e "s|\@MONIKER_LIBDIR\@|$(monikerdir)|" \
	$(srcdir)/Bonobo_Moniker_camera.oaf.in > Bonobo_Moniker_camera.oaf

@XML_I18N_MERGE_OAF_RULE@

CORBA_GENERATED =\
 GnoCam.h\
 GnoCam-common.c\
 GnoCam-stubs.c\
 GnoCam-skels.c

monikerdir = $(libdir)/bonobo/monikers
moniker_LTLIBRARIES=libmoniker_camera.la

libmoniker_camera_la_SOURCES = $(CORBA_GENERATED) bonobo-moniker-camera.c
libmoniker_camera_la_LDFLAGS = -module
libmoniker_camera_la_LIBADD = $(EXTRA_GNOME_LIBS)

$(CORBA_GENERATED): $(top_srcdir)/idl/GnoCam.idl
	$(ORBIT_IDL) $(top_srcdir)/idl/GnoCam.idl `$(GNOME_CONFIG) --cflags idl`

BUILT_SOURCES=$(CORBA_GENERATED)
CLEANFILES=$(CORBA_GENERATED) Bonobo_Moniker_camera.oaf
EXTRA_DIST = Bonobo_Moniker_camera.oaf.in



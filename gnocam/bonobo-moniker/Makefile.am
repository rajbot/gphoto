INCLUDES =\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 $(EXTRA_GNOME_CFLAGS)\
 $(GPHOTO_CFLAGS)

oafdir = $(datadir)/oaf
oaf_in_files = Bonobo_Moniker_camera.oaf.in
oaf_DATA = $(oaf_in_files:.oaf.in=.oaf)

@XML_I18N_MERGE_OAF_RULE@

CORBA_GENERATED =\
 GnoCam.h\
 GnoCam-common.c\
 GnoCam-stubs.c\
 GnoCam-skels.c

bin_PROGRAMS = \
 bonobo-moniker-camera

bonobo_moniker_camera_SOURCES =\
 $(CORBA_GENERATED)\
 bonobo-moniker-camera.h bonobo-moniker-camera.c

bonobo_moniker_camera_LDADD=\
 $(GPHOTO_LIBS)\
 $(EXTRA_GNOME_LIBS)

$(CORBA_GENERATED):
	$(ORBIT_IDL) $(top_srcdir)/idl/GnoCam.idl `$(GNOME_CONFIG) --cflags idl`

BUILT_SOURCES = $(CORBA_GENERATED)
CLEANFILES += $(BUILT_SOURCES)
EXTRA_DIST = $(oaf_in_files) $(oaf_DATA)



INCLUDES =\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 $(GPHOTO_CFLAGS)\
 $(GCONF_CFLAGS)\
 $(EXTRA_GNOME_CFLAGS)

bin_PROGRAMS = gnocam

CORBA_GENERATED =\
 GnoCam.h\
 GnoCam-common.c\
 GnoCam-stubs.c\
 GnoCam-skels.c

gnocam_SOURCES =\
 $(CORBA_GENERATED)\
 bonobo-storage-camera.c bonobo-storage-camera.h\
 bonobo-stream-camera.c bonobo-stream-camera.h\
 main.c\
 gnocam-main.c gnocam-main.h\
 gnocam-camera.c gnocam-camera.h

gnocam_LDADD =\
 $(GPHOTO_LIBS)\
 $(GCONF_LIBS)\
 $(EXTRA_GNOME_LIBS)

idls = $(top_srcdir)/idl/GnoCam.idl
idl_flags = `$(GNOME_CONFIG) --cflags idl` -D__GNOCAM_COMPILATION

$(CORBA_GENERATED): $(idls)
	$(ORBIT_IDL) $(top_srcdir)/idl/GnoCam.idl -I$(top_srcdir)/idl $(idl_flags)

GnoCam.h: $(idls)

BUILT_SOURCES=$(CORBA_GENERATED)
CLEANFILES += $(BUILT_SOURCES)

oafdir = $(datadir)/oaf
oaf_in_files = GNOME_GnoCam.oaf.in
oaf_DATA = $(oaf_in_files:.oaf.in=.oaf)
@XML_I18N_MERGE_OAF_RULE@

EXTRA_DIST = $(oaf_in_files)

dist-hook:
	cd $(distdir); rm -f $(BUILT_SOURCES)

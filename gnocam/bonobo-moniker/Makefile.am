INCLUDES =\
 -I$(top_srcdir)/libgnocam\
 -I.\
 -I$(srcdir) -I$(top_srcdir)\
 -I$(top_builddir)\
 -I$(includedir)\
 -DGNOCAM_DATADIR=\""$(datadir)"\"\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 -DGNOMELOCALEDIR=\""$(datadir)/locale"\"\
 $(EXTRA_WARNING_CFLAGS)\
 $(GCONF_CFLAGS)\
 $(GPHOTO_CFLAGS)\
 $(BONOBO_CFLAGS)

OAF_FILES =\
 Bonobo_Moniker_camera.oaf.in

oafdir = $(datadir)/oaf
oaf_DATA = $(OAF_FILES:.oaf.in=.oaf)

@XML_I18N_MERGE_OAF_RULE@

bin_PROGRAMS = \
 bonobo-moniker-camera

bonobo_moniker_camera_SOURCES =\
 bonobo-moniker-camera.h bonobo-moniker-camera.c

bonobo_moniker_camera_LDADD=\
 ../libgnocam/libgnocam.la\
 $(GPHOTO_LIBS)\
 $(BONOBO_LIBS)

EXTRA_DIST = Bonobo_Moniker_camera.oaf.in


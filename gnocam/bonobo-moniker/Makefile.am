INCLUDES =\
 -I$(top_srcdir)/libgnocam\
 -I.\
 -I$(srcdir) -I$(top_srcdir)\
 -I$(top_builddir)\
 -I$(includedir)\
 -DGNOCAM_DATADIR=\""$(datadir)"\"\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 -DGNOMELOCALEDIR=\""$(datadir)/locale"\"\
 $(EXTRA_GNOME_CFLAGS)\
 $(EXTRA_WARNING_CFLAGS)\
 $(GCONF_CFLAGS)\
 $(GPHOTO_CFLAGS)

OAF_FILES =\
 Bonobo_Moniker_camera.oaf.in

oafdir = $(datadir)/oaf
oaf_DATA = $(OAF_FILES:.oaf.in=.oaf)

@XML_I18N_MERGE_OAF_RULE@

bin_PROGRAMS = \
 bonobo-moniker-camera

bonobo_moniker_camera_SOURCES =\
 bonobo-moniker-camera.h bonobo-moniker-camera.c\
 e-clipped-label.h e-clipped-label.c\
 e-shell-constants.h\
 e-shell-folder-title-bar.h e-shell-folder-title-bar.c\
 e-title-bar.h e-title-bar.c\
 gnocam-capture.h gnocam-capture.c\
 gnocam-control.h gnocam-control.c\
 gnocam-file.h gnocam-file.c\
 gnocam-folder.h gnocam-folder.c\
 gnocam-storage-view.h gnocam-storage-view.c\
 utils.h utils.c

bonobo_moniker_camera_LDADD=\
 ../libgnocam/libgnocam.la\
 $(EXTRA_GNOME_LIBS)\
 $(GPHOTO_LIBS)

EXTRA_DIST = Bonobo_Moniker_camera.oaf.in


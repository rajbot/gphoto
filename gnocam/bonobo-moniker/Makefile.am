INCLUDES =\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 $(EXTRA_GNOME_CFLAGS)\
 $(GPHOTO_CFLAGS)

oafdir = $(datadir)/oaf
oaf_in_files = Bonobo_Moniker_camera.oaf.in
oaf_DATA = $(oaf_in_files:.oaf.in=.oaf)

@XML_I18N_MERGE_OAF_RULE@

noinst_LTLIBRARIES = libbonobo-storage-camera.la

libbonobo_storage_camera_la_SOURCES =\
 bonobo-storage-camera.h bonobo-storage-camera.c\
 bonobo-stream-camera.h bonobo-stream-camera.c

bin_PROGRAMS = \
 bonobo-moniker-camera

bonobo_moniker_camera_SOURCES =\
 bonobo-moniker-camera.h bonobo-moniker-camera.c\
 bonobo-storage-camera.h bonobo-storage-camera.c\
 bonobo-stream-camera.h bonobo-stream-camera.c

bonobo_moniker_camera_LDADD=\
 ../libgnocam/libgnocam.la\
 $(GPHOTO_LIBS)\
 $(EXTRA_GNOME_LIBS)

EXTRA_DIST = $(oaf_in_files) $(oaf_DATA)


PLUGIN_DIR=$(libdir)/bonobo/plugin

INCLUDES = \
 -I$(top_srcdir)/libgnocam\
 -I$(srcdir)\
 -I$(top_srcdir)\
 -I$(top_builddir)\
 -I$(includedir)\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 -DGNOMELOCALEDIR=\""$(datadir)/locale"\"\
 $(EXTRA_GNOME_CFLAGS)\
 $(GCONF_CFLAGS)\
 $(EXTRA_WARNING_CFLAGS)\
 $(GPHOTO_CFLAGS)\
 $(BONOBO_CFLAGS)

common_ldflags = -avoid-version

bonobo_plugindir = $(PLUGIN_DIR)
bonobo_plugin_LTLIBRARIES = libstorage_camera.la

libstorage_camera_la_SOURCES =\
 bonobo-storage-camera.h bonobo-storage-camera.c\
 bonobo-stream-camera.h bonobo-stream-camera.c

libstorage_camera_la_LDFLAGS =\
 $(GPHOTO_LIBS)\
 $(GCONF_LIBS)\
 $(common_ldflags)

libstorage_camera_la_LIBADD =\
 ../libgnocam/libgnocam.la\
 $(GNOME_LIBS)\
 $(GNOME_LIBDIR)\
 $(GCONF_LIBS)\
 $(EXTRA_GNOME_LIBS)\
 $(GPHOTO_LIBS)\
 $(BONOBO_LIBS)

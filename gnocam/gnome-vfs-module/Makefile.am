
NULL =

modulesdir = $(libdir)/vfs/modules

modules_LTLIBRARIES =\
 libvfs-camera.la\
 $(NULL)

libvfs_camera_la_SOURCES =\
 camera-method.c\
 utils.h utils.c\
 $(NULL)

INCLUDES =\
 -I$(top_srcdir)\
 -I$(top_srcdir)/libgnocam\
 -D_POSIX_PTHREAD_SEMANTICS\
 $(GPHOTO_CFLAGS)\
 $(GCONF_CFLAGS)\
 $(GNOME_INCLUDEDIR)\
 $(EXTRA_WARNING_CFLAGS)\
 $(GNOME_CFLAGS)\
 $(VFS_CFLAGS)\
 $(BONOBO_CFLAGS)\
 $(NULL)

libvfs_camera_la_LIBADD =\
 ../libgnocam/libgnocam.la\
 $(GNOME_LIBS)\
 $(GNOME_LIBDIR)\
 $(GNOME_LIBS)\
 $(INTLLIBS)\
 $(GPHOTO_LIBS)\
 $(VFS_LIBS)\
 $(BONOBO_LIBS)\
 $(NULL)

libvfs_camera_la_LDFLAGS =\
 -export_dynamic\
 -avoid-version

modulesconfdir = $(sysconfdir)/vfs/modules
modulesconf_DATA = camera-methods.conf

EXTRA_DIST = camera-methods.conf


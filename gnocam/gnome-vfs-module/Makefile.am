
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
 $(GPHOTO_CFLAGS)\
 $(EXTRA_WARNING_CFLAGS)\
 $(VFS_CFLAGS)\
 $(GNOME_INCLUDEDIR)\
 $(NULL)

libvfs_camera_la_LIBADD =\
 ../libgnocam/libgnocam.la\
 $(VFS_LIBS)\
 $(NULL)

libvfs_camera_la_LDFLAGS =\
 -export_dynamic\
 -avoid-version

modulesconfdir = $(sysconfdir)/vfs/modules
modulesconf_DATA = camera-methods.conf

EXTRA_DIST = camera-methods.conf


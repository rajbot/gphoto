modulesdir = $(libdir)/gnome-vfs-2.0/modules
modules_LTLIBRARIES=libcamera.la

libcamera_la_SOURCES =			\
	camera-method.c			\
	camera-dir.h camera-dir.c	\
	camera-file.h camera-file.c	\
	camera-utils.h camera-utils.c

INCLUDES = -DG_LOG_DOMAIN=\"camera\" $(VFS_CFLAGS)	\
	-I$(top_srcdir) -I$(top_builddir)
libcamera_la_LIBADD = $(top_builddir)/libgnocam/libgnocam.la $(VFS_LIBS)
libcamera_la_LDFLAGS = -export_dynamic -avoid-version

modulesconfdir = $(sysconfdir)/gnome-vfs-2.0/modules
modulesconf_DATA=camera-methods.conf

EXTRA_DIST=camera-methods.conf

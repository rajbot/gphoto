
modulesdir = $(libdir)/vfs/modules

modules_LTLIBRARIES = libvfs-camera.la

libvfs_camera_la_SOURCES = camera-method.c

INCLUDES =\
 $(VFS_CFLAGS)

libvfs_camera_la_LIBADD =\
 $(VFS_LIBS)

libvfs_camera_la_LDFLAGS = -export_dynamic -avoid-version

modulesconfdir = $(sysconfdir)/vfs/modules
modulesconf_DATA = camera-methods.conf

EXTRA_DIST = camera-methods.conf


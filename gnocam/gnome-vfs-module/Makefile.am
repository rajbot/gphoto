
modulesdir = $(libdir)/vfs/modules

if VFS_MODULE
modules_LTLIBRARIES=libvfs-camera.la
else
modules_LTLIBRARIES=
endif

libvfs_camera_la_SOURCES = camera-method.c

INCLUDES =\
 -DG_LOG_DOMAIN=\"libvfs-camera\"\
 $(GCONF_CFLAGS)\
 $(VFS_CFLAGS)\
 $(GPHOTO_CFLAGS)

libvfs_camera_la_LIBADD = $(VFS_LIBS) $(GCONF_LIBS) $(GPHOTO_LIBS)

libvfs_camera_la_LDFLAGS = -export_dynamic -avoid-version

modulesconfdir = $(sysconfdir)/vfs/modules
if VFS_MODULE
modulesconf_DATA=camera-methods.conf
endif

EXTRA_DIST=camera-methods.conf


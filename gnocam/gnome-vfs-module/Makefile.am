
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
 -D_POSIX_PTHREAD_SEMANTICS\
 $(GLIB_CFLAGS)\
 $(NULL)

libvfs_camera_la_LIBADD =\
 $(NULL)

libvfs_camera_la_LDFLAGS =\
 -export_dynamic\
 -avoid-version

CFLAGS = @CFLAGS@\
 `gphoto2-config --cflags`\
 `gpio-config --cflags`\
 `gnome-config --cflags vfs`\
 `gconf-config --cflags`\
 `xml-config --cflags`\
 `glib-config --cflags`

LDFLAGS =@LDFLAGS@\
 `gphoto2-config --libs`\
 `gpio-config --libs`\
 `gnome-config --libs vfs`\
 `gconf-config --libs`\
 `xml-config --libs`\
 `glib-config --libs`

modulesconfdir = $(sysconfdir)/vfs/modules
modulesconf_DATA = camera-methods.conf

EXTRA_DIST = camera-methods.conf


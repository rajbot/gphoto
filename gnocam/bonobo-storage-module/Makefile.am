PLUGIN_DIR=$(libdir)/bonobo/plugin

INCLUDES = \
 -I$(srcdir)\
 -I$(top_srcdir)\
 -I$(top_builddir)\
 -I$(includedir)\
 $(GNOME_INCLUDEDIR)

CFLAGS = @CFLAGS@\
 `gphoto2-port-config --cflags`\
 `gphoto2-config --cflags`\
 `gconf-config --cflags`\
 `xml-config --cflags`

LDFLAGS = @LDFLAGS@\
 `gnome-config --libs libglade gdk_pixbuf gnomeui vfs xml bonobox gal`\
 `gconf-config --libs gconf gconf-gtk`

common_ldflags = -avoid-version

bonobo_plugindir = $(PLUGIN_DIR)
bonobo_plugin_LTLIBRARIES = libstorage_camera.la

libstorage_camera_la_SOURCES =\
 bonobo-storage-camera.h bonobo-storage-camera.c\
 bonobo-stream-camera.h bonobo-stream-camera.c\
 utils.h utils.c

libstorage_camera_la_LDFLAGS =\
 $(GPHOTO2_LIBS)\
 $(GCONF_LIBS)\
 $(VFS_LIBS)\
 $(common_ldflags)

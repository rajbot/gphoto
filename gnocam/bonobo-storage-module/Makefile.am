PLUGIN_DIR=$(libdir)/bonobo/plugin

INCLUDES = \
 -DGNOMELOCALEDIR=\""$(datadir)/locale"\"\
 -I$(srcdir) -I$(top_srcdir)\
 -I$(top_builddir)\
 -DPLUGIN_DIR=\""$(PLUGIN_DIR)"\"\
 -I$(includedir)\
 $(GNOME_INCLUDEDIR)

common_ldflags = -avoid-version

bonobo_plugindir = $(PLUGIN_DIR)
bonobo_plugin_LTLIBRARIES = libstorage_vfs.la

libstorage_vfs_la_SOURCES =\
 bonobo-storage-vfs.h bonobo-storage-vfs.c\
 bonobo-stream-vfs.h bonobo-stream-vfs.c

libstorage_vfs_la_LDFLAGS =\
 $(common_ldflags)

libstorage_vfs_la_LIBADD =

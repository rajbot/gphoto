INCLUDES =\
 -DG_LOG_DOMAIN=\"GnoCam\"\
 $(GPHOTO_CFLAGS)\
 $(EXTRA_GNOME_CFLAGS)\
 $(GCONF_CFLAGS)\
 $(VFS_CFLAGS)

noinst_LTLIBRARIES = libgnocam.la

libgnocam_la_SOURCES =\
 gphoto-extensions.h gphoto-extensions.c

libgnocam_la_LIBADD =\
 $(GPHOTO_LIBS)\
 $(EXTRA_GNOME_LIBS)\
 $(GCONF_LIBS)\
 $(VFS_LIBS)


## Process this file with automake to produce Makefile.in

INCLUDES = \
 -DIMAGEDIR=\"$(prefix)/share/pixmaps\"\
 -DGNOME_LOCALEDIR=\""$(datadir)/locale"\"\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 -I$(top_srcdir)/libgnocam\
 $(CAPPLET_CFLAGS)\
 $(GCONF_CFLAGS)\
 $(GPHOTO_CFLAGS)

bin_PROGRAMS=camera-capplet

camera_capplet_SOURCES =\
 gnocam-capplet-content.c gnocam-capplet-content.h\
 main.c

camera_capplet_LDADD =\
 $(top_builddir)/libgnocam/libgnocam.a\
 $(CAPPLET_LIBS)\
 $(GCONF_LIBS)\
 $(GPHOTO_LIBS)

sysdir=$(datadir)/control-center/Peripherals
sys_DATA=camera-capplet.desktop

deskdir=$(datadir)/gnome/apps/Settings/Peripherals
desk_DATA=camera-capplet.desktop

EXTRA_DIST=camera-capplet.desktop

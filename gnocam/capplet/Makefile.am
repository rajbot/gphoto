## Process this file with automake to produce Makefile.in

INCLUDES = \
 -DIMAGEDIR=\"$(prefix)/usr/share/pixmaps\"\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 $(GCONF_CFLAGS)\
 $(CAPPLET_CFLAGS)\
 $(EXTRA_GNOME_CFLAGS)\
 $(GPHOTO_CFLAGS)

bin_PROGRAMS = camera-capplet

camera_capplet_SOURCES =\
 gnocam-capplet-content.c gnocam-capplet-content.h\
 gnocam-capplet-model.c gnocam-capplet-model.h\
 main.c

camera_capplet_LDADD =\
 $(CAPPLET_LIBS)\
 $(GCONF_LIBS)\
 $(EXTRA_GNOME_LIBS)\
 $(GPHOTO_LIBS)

EXTRA_DIST = camera-capplet.desktop

sysdir = $(datadir)/control-center/Peripherals
sys_DATA = camera-capplet.desktop

deskdir = $(datadir)/gnome/apps/Settings/Peripherals
desk_DATA = camera-capplet.desktop


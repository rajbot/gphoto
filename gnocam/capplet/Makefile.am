## Process this file with automake to produce Makefile.in

INCLUDES = \
 -DIMAGEDIR=\"$(prefix)/share/pixmaps\"\
 -DGNOME_LOCALEDIR=\""$(datadir)/locale"\"\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 -I$(top_srcdir)/libgnocam\
 $(CAPPLET_CFLAGS)\
 $(GPHOTO_CFLAGS)

if CAPPLET
bin_PROGRAMS=camera-capplet
else
bin_PROGRAMS=
endif

camera_capplet_SOURCES =\
 gnocam-capplet-content.c gnocam-capplet-content.h\
 main.c

camera_capplet_LDADD =\
 $(top_builddir)/libgnocam/libgnocam.a\
 $(CAPPLET_LIBS)\
 $(GPHOTO_LIBS)

sysdir=$(datadir)/control-center/Peripherals
if CAPPLET
sys_DATA=camera-capplet.desktop
endif

deskdir=$(datadir)/gnome/apps/Settings/Peripherals
if CAPPLET
desk_DATA=camera-capplet.desktop
endif

EXTRA_DIST=camera-capplet.desktop

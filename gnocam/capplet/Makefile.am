## Process this file with automake to produce Makefile.in

INCLUDES = \
 -I$(top_srcdir)/libgnocam\
 -I$(top_srcdir)/intl\
 -DGNOME_LOCALEDIR=\""$(datadir)/locale"\"\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 $(CAPPLET_CFLAGS)\
 $(GAL_CFLAGS)\
 $(GNOME_INCLUDEDIR)\
 $(GCONF_CFLAGS)\
 $(GPHOTO_CFLAGS)

bin_PROGRAMS = camera-capplet

camera_capplet_SOURCES =\
 gnocam-capplet-content.c gnocam-capplet-content.h\
 main.c

camera_capplet_LDADD =\
 ../libgnocam/libgnocam.la\
 $(CAPPLET_LIBS)\
 $(GAL_LIBS)\
 $(INTLLIBS)

EXTRA_DIST = camera-capplet.desktop

sysdir = $(datadir)/control-center/Peripherals
sys_DATA = camera-capplet.desktop

install-data-local:
	$(INSTALL_DATA) $(srcdir)/camera-capplet.desktop $(DESTDIR)$(datadir)/gnome/apps/Settings/Peripherals/camera-capplet.desktop


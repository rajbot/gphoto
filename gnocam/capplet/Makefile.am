## Process this file with automake to produce Makefile.in

INCLUDES = \
 -I$(top_srcdir)/intl\
 $(GNOME_INCLUDEDIR)

bin_PROGRAMS = camera-capplet

camera_capplet_SOURCES =\
 camera-capplet.c\
 capplet.h capplet.c\
 capplet-callbacks.h capplet-callbacks.c\
 preferences.h preferences.c

camera_capplet_LDADD =\
 $(GNOME_LIBDIR)\
 $(GNOMEUI_LIBS)\
 $(INTLLIBS)

EXTRA_DIST = camera-capplet.desktop

sysdir = $(datadir)/control-center/Peripherals
sys_DATA = camera-capplet.desktop

install-data-local:
	$(INSTALL_DATA) $(srcdir)/camera-capplet.desktop $(DESTDIR)$(datadir)/gnome/apps/Settings/Peripherals/camera-capplet.desktop


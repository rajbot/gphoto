
SUBDIRS = ui

INCLUDES = \
 -I$(top_srcdir)/libgnocam\
 -DGNOME_LOCALEDIR=\""$(datadir)/locale"\"\
 -DGNOCAM_GLADEDIR=\""$(gladedir)"\"\
 -DGNOCAM_DATADIR=\""$(datadir)"\"\
 -DGNOCAM_ICONSDIR=\""$(iconsdir)"\"\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 $(GCONF_CFLAGS)\
 $(EXTRA_WARNING_CFLAGS)\
 $(EXTRA_GNOME_CFLAGS)\
 $(GPHOTO_CFLAGS)

bin_PROGRAMS = gnocam

gnocam_SOURCES =\
 cameras.h cameras.c\
 capture.h capture.c\
 file-operations.h file-operations.c\
 frontend.h frontend.c\
 gnocam.h gnocam.c\
 gnocam-shortcut-bar.h gnocam-shortcut-bar.c\
 preferences.h preferences.c

gnocam_LDFLAGS =\
 $(GPHOTO2_LIBS)\
 $(GPIO_LIBS)

gladedir = $(datadir)/$(PACKAGE)/
glade_DATA = gnocam.glade

iconsdir = $(datadir)/images/$(PACKAGE)

gnocam_LDADD =\
 $(EXTRA_GNOME_LIBS)\
 $(INTLLIBS)\
 $(GCONF_LIBS)\
 $(GPHOTO_LIBS)\
 ../libgnocam/libgnocam.la

EXTRA_DIST = $(glade_DATA)


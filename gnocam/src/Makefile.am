
SUBDIRS = ui

gladedir = $(datadir)/$(PACKAGE)/

INCLUDES = \
 -I$(top_srcdir)/libgnocam\
 -DGNOME_LOCALEDIR=\""$(datadir)/locale"\"\
 -DGNOCAM_GLADEDIR=\""$(gladedir)"\"\
 -DGNOCAM_DATADIR=\""$(datadir)"\"\
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
 preferences.h preferences.c

gnocam_LDFLAGS =\
 $(GPHOTO2_LIBS)\
 $(GPIO_LIBS)

glade_DATA = gnocam.glade

gnocam_LDADD =\
 $(EXTRA_GNOME_LIBS)\
 $(INTLLIBS)\
 $(GCONF_LIBS)\
 $(GPHOTO_LIBS)\
 ../libgnocam/libgnocam.la

EXTRA_DIST = $(glade_DATA)


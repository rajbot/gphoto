
gladedir = $(datadir)/$(PACKAGE)/

INCLUDES = \
 -DGNOME_LOCALEDIR=\""$(datadir)/locale"\"\
 -DLOCALEDIR=\"$(datadir)/locale\"\
 -DGNOCAM_GLADEDIR=\""$(gladedir)"\"\
 $(GNOME_INCLUDEDIR)\
 -I$(top_srcdir)/src\
 -I$(includedir)/src

CFLAGS = @CFLAGS@\
 `gphoto2-config --cflags`\
 `gpio-config --cflags`\
 `gnome-config --cflags libglade gdk_pixbuf gnomeui`

LDFLAGS = @LDFLAGS@\
 `gphoto2-config --libs`\
 `gpio-config --libs`\
 `gnome-config --libs libglade gdk_pixbuf gnomeui`

bin_PROGRAMS = gnocam

gnocam_SOURCES =\
 gnocam.h gnocam.c\
 callbacks.c\
 camera_properties.h camera_properties.c\
 preferences.h preferences.c\
 save.h save.c

glade_DATA = gnocam.glade

gnocam_LDADD = \
 $(GNOME_LIBDIR)\
 $(GNOMEUI_LIBS)\
 $(INTLLIBS)

EXTRA_DIST = $(glade_DATA)

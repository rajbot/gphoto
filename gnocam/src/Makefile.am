
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
 `gnome-config --cflags libglade gdk_pixbuf gnomeui vfs`\
 `gconf-config --cflags gconf gconf-gtk`

LDFLAGS = @LDFLAGS@\
 `gphoto2-config --libs`\
 `gpio-config --libs`\
 `gnome-config --libs libglade gdk_pixbuf gnomeui vfs`\
 `gconf-config --libs gconf gconf-gtk`

bin_PROGRAMS = gnocam

gnocam_SOURCES =\
 callbacks.h callbacks.c\
 cameras.h cameras.c\
 frontend.h frontend.c\
 gnocam.h gnocam.c\
 gphoto-extensions.h gphoto-extensions.c\
 information.h information.c\
 notification.h notification.c\
 preferences.h preferences.c\
 preview.h preview.c\
 properties.h properties.c\
 save.h save.c

glade_DATA = gnocam.glade

gnocam_LDADD = \
 $(GNOME_LIBDIR)\
 $(GNOMEUI_LIBS)\
 $(INTLLIBS)

EXTRA_DIST = $(glade_DATA)

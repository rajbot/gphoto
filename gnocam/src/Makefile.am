
SUBDIRS = ui

gladedir = $(datadir)/$(PACKAGE)/
helpdir = $(datadir)/gnome/help/$(PACKAGE)/C

INCLUDES = \
 -DGNOME_LOCALEDIR=\""$(datadir)/locale"\"\
 -DLOCALEDIR=\"$(datadir)/locale\"\
 -DGNOCAM_GLADEDIR=\""$(gladedir)"\"\
 -DGNOCAM_HELPDIR=\""$(helpdir)"\"\
 $(GNOME_INCLUDEDIR)\
 -I$(top_srcdir)/src\
 -I$(includedir)/src

CFLAGS = @CFLAGS@\
 `gnome-config --cflags libglade gdk_pixbuf gnomeui vfs xml bonobox gal`\
 `gconf-config --cflags gconf gconf-gtk`\
 `gphoto2-config --cflags`\
 `gpio-config --cflags` 

LDFLAGS = @LDFLAGS@\
 `gnome-config --libs libglade gdk_pixbuf gnomeui vfs xml bonobox gal`\
 `gconf-config --libs gconf gconf-gtk`

bin_PROGRAMS = gnocam

gnocam_SOURCES =\
 cameras.h cameras.c\
 file-operations.h file-operations.c\
 frontend.h frontend.c\
 gnocam.h gnocam.c\
 gphoto-extensions.h gphoto-extensions.c\
 preferences.h preferences.c\
 preview.h preview.c\
 utils.h utils.c

gnocam_LDFLAGS =\
 $(GPHOTO2_LIBS)\
 $(GPIO_LIBS)

glade_DATA = gnocam.glade

gnocam_LDADD =\
 $(GNOME_LIBDIR)\
 $(GNOMEUI_LIBS)\
 $(INTLLIBS)

EXTRA_DIST = $(glade_DATA)


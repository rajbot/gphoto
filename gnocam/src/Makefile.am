
SUBDIRS = ui

gladedir = $(datadir)/$(PACKAGE)/

INCLUDES = \
 -DGNOME_LOCALEDIR=\""$(datadir)/locale"\"\
 -DLOCALEDIR=\"$(datadir)/locale\"\
 -DGNOCAM_GLADEDIR=\""$(gladedir)"\"\
 $(GNOME_INCLUDEDIR)\
 -I$(top_srcdir)/src\
 -I$(includedir)/src

CFLAGS = @CFLAGS@\
 `gnome-config --cflags libglade gdk_pixbuf gnomeui vfs xml gtkhtml bonobox`\
 `gconf-config --cflags gconf gconf-gtk`\
 `gphoto2-config --cflags`\
 `gpio-config --cflags` 

LDFLAGS = @LDFLAGS@\
 `gnome-config --libs libglade gdk_pixbuf gnomeui vfs xml gtkhtml bonobox`\
 `gconf-config --libs gconf gconf-gtk`

bin_PROGRAMS = gnocam

gnocam_SOURCES =\
 callbacks.c\
 cameras.h cameras.c\
 file-operations.h file-operations.c\
 frontend.h frontend.c\
 gallery.h gallery.c\
 gnocam.h gnocam.c\
 gphoto-extensions.h gphoto-extensions.c\
 notification.h notification.c\
 preferences.h preferences.c\
 preview.h preview.c\
 properties.h properties.c\
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

BUILT_SOURCES =\
 libglade-i18n.c

libglade-i18n.c: gnocam.glade
	libglade-xgettext -c -o libglade-i18n.c gnocam.glade

CLEANFILES += libglade-i18n.c


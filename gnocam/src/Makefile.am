
INCLUDES = \
 -DGNOME_LOCALEDIR=\""$(datadir)/locale"\"\
 -DGNOCAM_DATADIR=\""$(datadir)"\"\
 -DIMAGEDIR=\""$(datadir)/pixmaps"\"\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 $(EXTRA_GNOME_CFLAGS)\
 $(GCONF_CFLAGS)\
 $(GPHOTO_CFLAGS)

bin_PROGRAMS = gnocam

gnocam_SOURCES =\
 e-clipped-label.h e-clipped-label.c\
 e-shell-constants.h\
 e-shell-folder-title-bar.h e-shell-folder-title-bar.c\
 e-title-bar.h e-title-bar.c\
 gnocam.c\
 gnocam-configuration.h gnocam-configuration.c\
 gnocam-camera.h gnocam-camera.c\
 gnocam-capture.h gnocam-capture.c\
 gnocam-file.h gnocam-file.c\
 gnocam-folder.h gnocam-folder.c\
 gnocam-main.h gnocam-main.c\
 gnocam-preferences.h gnocam-preferences.c\
 gnocam-storage-view.h gnocam-storage-view.c\
 utils.h utils.c\
 gnocam-shortcut-bar.h gnocam-shortcut-bar.c

gnocam_LDFLAGS =\
 $(GPHOTO2_LIBS)\
 $(GPIO_LIBS)\
 $(EXTRA_GNOME_LIBS)\
 $(GCONF_LIBS)

iconsdir = $(datadir)/images/$(PACKAGE)

gnocam_LDADD =\
 $(INTLLIBS)\
 $(GPHOTO_LIBS)\
 ../libgnocam/libgnocam.la\
 ../bonobo-moniker/libbonobo-storage-camera.la


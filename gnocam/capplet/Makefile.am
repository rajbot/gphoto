INCLUDES = 					\
 -DIMAGEDIR=\"$(prefix)/share/pixmaps\"		\
 -DGNOME_LOCALEDIR=\""$(datadir)/locale"\"	\
 -DG_LOG_DOMAIN=\"$(PACKAGE)\"\
 $(GNOCAM_CAPPLET_CFLAGS)

bin_PROGRAMS=camera-capplet
camera_capplet_LDADD = $(GNOCAM_CAPPLET_LIBS)
camera_capplet_SOURCES = gnocam-capplet.c gnocam-capplet.h main.c

@INTLTOOL_DESKTOP_RULE@

desktopdir = $(datadir)/@GETTEXT_PACKAGE@/capplets
Desktop_in_files = camera-capplet.desktop.in
desktop_DATA = $(Desktop_in_files:.desktop.in=.desktop)

sysdir=$(datadir)/control-center/Peripherals
sys_DATA=camera-capplet.desktop

deskdir=$(datadir)/gnome/apps/Settings/Peripherals
desk_DATA=camera-capplet.desktop

$(Desktop_in_files): $(Desktop_in_files:.desktop.in=.desktop.in.in)
	sed -e "s|\@PREFIX\@|$(prefix)|" $< > $@

CLEANFILES = camera-capplet.desktop.in camera-capplet.desktop
EXTRA_DIST = $(Desktop_in_files) camera-capplet.desktop.in.in

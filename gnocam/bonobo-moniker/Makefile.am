INCLUDES =\
 -DGNOMELOCALEDIR=\""$(datadir)/locale"\"\
 -I.\
 -I$(srcdir) -I$(top_srcdir)\
 -I$(top_builddir)\
 -I$(top_srcdir)/bonobo\
 -I$(includedir)

OAF_FILES =\
 Bonobo_Moniker_camera.oaf.in

oafdir = $(datadir)/oaf
oaf_DATA = $(OAF_FILES:.oaf.in=.oaf)

@XML_I18N_MERGE_OAF_RULE@

CFLAGS = @CFLAGS@\
 `gphoto2-port-config --cflags`\
 `gphoto2-config --cflags`\
 `gconf-config --cflags`\
 `gnome-config --cflags bonobo bonobox xml vfs`\
 `xml-config --cflags`
    
bin_PROGRAMS = \
 bonobo-moniker-camera

bonobo_moniker_camera_SOURCES =\
 bonobo-moniker-camera.h bonobo-moniker-camera.c\
 gnocam-control.h gnocam-control.c\
 utils.h utils.c

bonobo_moniker_camera_LDADD=\
 $(GPHOTO2_LIBS)\
 $(VFS_LIBS)\
 $(BONOBO_LIBS)



SUBDIRS = ui

gladedir = $(datadir)/$(PACKAGE)/

GPHOTO_CORBA_GENERATED =\
 Gphoto.h\
 Gphoto-common.c\
 Gphoto-skels.c\
 Gphoto-stubs.c

INCLUDES = \
 -DGNOME_LOCALEDIR=\""$(datadir)/locale"\"\
 -DGNOCAM_GLADEDIR=\""$(gladedir)"\"\
 -DGNOCAM_DATADIR=\""$(datadir)"\"

CFLAGS = @CFLAGS@\
 `gnome-config --cflags libglade gdk_pixbuf gnomeui vfs bonobox gal`\
 `gconf-config --cflags gconf gconf-gtk`\
 `gphoto2-config --cflags`\
 `gphoto2-port-config --cflags`

LDFLAGS = @LDFLAGS@\
 `gnome-config --libs libglade gdk_pixbuf gnomeui vfs bonobox gal`\
 `gconf-config --libs gconf gconf-gtk`

bin_PROGRAMS = gnocam

idls =\
 $(top_srcdir)/idl/Gphoto.idl

gnocam_SOURCES =\
 $(GPHOTO_CORBA_GENERATED)\
 cameras.h cameras.c\
 capture.h capture.c\
 file-operations.h file-operations.c\
 frontend.h frontend.c\
 gnocam.h gnocam.c\
 gphoto-extensions.h gphoto-extensions.c\
 preferences.h preferences.c

gnocam_LDFLAGS =\
 $(GPHOTO2_LIBS)\
 $(GPIO_LIBS)

glade_DATA = gnocam.glade

gnocam_LDADD =\
 $(GNOME_LIBDIR)\
 $(GNOMEUI_LIBS)\
 $(INTLLIBS)

idl_flags = `$(GNOME_CONFIG) --cflags idl`

$(GPHOTO_CORBA_GENERATED): $(idls)
	$(ORBIT_IDL) -I$(srcdir) -I$(datadir)/idl $(idl_flags) $(top_srcdir)/idl/Gphoto.idl

BUILT_SOURCES = $(GPHOTO_CORBA_GENERATED)
EXTRA_DIST = $(glade_DATA)


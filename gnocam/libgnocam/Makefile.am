INCLUDES =\
 -DGNOMELOCALEDIR=\""$(datadir)/locale"\"\
 -DG_LOG_DOMAIN=\"GnoCam\"\
 -DDATADIR=\""$(datadir)"\"\
 -I.\
 -I$(srcdir) -I$(top_srcdir)\
 -I$(top_srcdir)/src\
 -I$(top_builddir)\
 -I$(includedir)\
 $(GCONF_CFLAGS)\
 $(GNOME_INCLUDEDIR)\
 $(EXTRA_WARNING_CFLAGS)\
 $(GNOME_CFLAGS)\
 $(GPHOTO_CFLAGS)
	

CORBA_SOURCES =\
 GnoCam.h\
 GnoCam-common.c\
 GnoCam-stubs.c\
 GnoCam-skels.c

noinst_LTLIBRARIES = libgnocam.la

libgnocam_la_SOURCES =\
 $(CORBA_SOURCES)\
 gphoto-extensions.h gphoto-extensions.c

libgnocam_la_LIBADD =\
 $(GNOME_LIBS)\
 $(GNOME_LIBDIR)\
 $(GNOME_LIBS)\
 $(OAF_LIBS)\
 $(GCONF_LIBS)

idls = $(top_srcdir)/idl/GnoCam.idl

idl_flags = `$(GNOME_CONFIG) --cflags idl` -D__GNOCAM_COMPILATION

$(CORBA_SOURCES): $(idls)
	$(ORBIT_IDL) $(top_srcdir)/idl/GnoCam.idl -I$(top_srcdir)/idl $(idl_flags)

GnoCam.h: $(idls)

BUILT_SOURCES=$(CORBA_SOURCES)

CLEANFILES += $(BUILT_SOURCES)


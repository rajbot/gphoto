INCLUDES =\
 $(CAPPLET_CFLAGS)\
 $(GCONF_CFLAGS)\
 $(EXTRA_GNOME_CFLAGS)\
 $(GPHOTO_CFLAGS)

noinst_LIBRARIES = libgnocam.a

CORBA_GENERATED =\
 GnoCam.h\
 GnoCam-common.c\
 GnoCam-skels.c\
 GnoCam-stubs.c

libgnocam_a_SOURCES =\
 $(CORBA_GENERATED)\
 gnocam-capplet-table-scrolled.c gnocam-capplet-table-scrolled.h\
 gnocam-capplet-table.c gnocam-capplet-table.h\
 gnocam-capplet-model.c gnocam-capplet-model.h

$(CORBA_GENERATED): $(top_srcdir)/idl/GnoCam.idl
	$(ORBIT_IDL) $(top_srcdir)/idl/GnoCam.idl `$(GNOME_CONFIG) --cflags idl`

BUILT_SOURCES += $(CORBA_GENERATED)
CLEANFILES += $(CORBA_GENERATED)


INCLUDES =\
 $(VFS_CFLAGS)\
 $(EXTRA_GNOME_CFLAGS)\
 -DG_LOG_DOMAIN=\"gnocam-test\"

if HAVE_GNOCAM
GNOCAM_PROGS=test-gnocam
endif

noinst_PROGRAMS =\
 test-vfs-module\
 $(GNOCAM_PROGS)\
 test-moniker

test_moniker_SOURCE = test-moniker.c
test_moniker_LDADD =\
 $(EXTRA_GNOME_LIBS)

test_vfs_module_SOURCE = test-vfs-module.c
test_vfs_module_LDADD =\
 $(VFS_LIBS)\
 $(EXTRA_GNOME_LIBS)

if HAVE_GNOCAM
CORBA_GENERATED =\
 GnoCam.h\
 GnoCam-common.c\
 GnoCam-skels.c\
 GnoCam-stubs.c

test_gnocam_SOURCES =\
 $(CORBA_GENERATED)\
 test-gnocam.c
test_gnocam_LDADD =\
 $(EXTRA_GNOME_LIBS)

$(CORBA_GENERATED): $(top_srcdir)/idl/GnoCam.idl
	$(ORBIT_IDL) $(top_srcdir)/idl/GnoCam.idl `$(GNOME_CONFIG) --cflags idl`
endif

BUILT_SOURCES=$(CORBA_GENERATED)
CLEANFILES=$(BUILT_SOURCES)


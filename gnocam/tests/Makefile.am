INCLUDES =				\
 $(TEST_GNOCAM_CFLAGS)			\
 -DG_LOG_DOMAIN=\"gnocam-test\"

CORBA_GENERATED = GnoCam.h GnoCam-common.c GnoCam-stubs.c

$(CORBA_GENERATED): $(top_srcdir)/idl/GnoCam.idl
	$(ORBIT_IDL) --noskels \
		-I`$(PKG_CONFIG) --variable=idldir libbonobo-2.0` \
		-I`$(PKG_CONFIG) --variable=idldir bonobo-activation-2.0` \
		$(top_srcdir)/idl/GnoCam.idl

BUILT_SOURCES = $(CORBA_GENERATED)
CLEANFILES    = $(CORBA_GENERATED)

noinst_PROGRAMS = test-gnocam
# test-vfs-module\
# test-moniker

test_gnocam_SOURCES = $(CORBA_GENERATED) test-gnocam.c
test_gnocam_LDADD = $(TEST_GNOCAM_LIBS)

#test_moniker_SOURCES = test-moniker.c
#test_moniker_LDADD =

#test_vfs_module_SOURCES = test-vfs-module.c
#test_vfs_module_LDADD = $(VFS_LIBS)

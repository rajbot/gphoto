INCLUDES = -I$(top_srcdir) $(TEST_CFLAGS) $(VFS_CFLAGS)

noinst_PROGRAMS = test-libgnocam test-vfs

test_libgnocam_LDADD = $(top_builddir)/libgnocam/libgnocam.la $(TEST_LIBS)
test_vfs_LDADD = $(VFS_LIBS)

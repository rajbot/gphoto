INCLUDES = -I$(top_srcdir) $(TEST_GKNC_CFLAGS)

if HAVE_TEST_GKNC
noinst_PROGRAMS = test-gknc
endif

test_gknc_SOURCES = test-gknc.c
test_gknc_LDADD =				\
	$(top_builddir)/libgknc/libgknc.la	\
	$(top_builddir)/libgpknc/libgpknc.la	\
	$(TEST_GKNC_LIBS)

INCLUDES = $(GKNC_CFLAGS) $(GPKNC_CFLAGS) -I$(top_srcdir)

if HAVE_GKNC
if HAVE_GPKNC
noinst_PROGRAMS = test-gknc
endif
endif

test_gknc_SOURCES = test-gknc.c
test_gknc_LDADD =				\
	$(top_builddir)/libgknc/libgknc.la	\
	$(top_builddir)/libgpknc/libgpknc.la	\
	$(GKNC_LIBS) $(GPKNC_LIBS)

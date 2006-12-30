INCLUDES = -I$(top_srcdir) $(GPKNC_CFLAGS)

if HAVE_GPKNC
noinst_PROGRAMS = test-gpknc
endif

test_gpknc_SOURCES = test-gpknc.c
test_gpknc_LDADD =				\
	$(top_builddir)/libknc/libknc.la	\
	$(top_builddir)/libgpknc/libgpknc.la	\
	$(GPKNC_LIBS) $(INTLLIBS)

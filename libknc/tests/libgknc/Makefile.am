INCLUDES = $(GKNC_CFLAGS)

if HAVE_GKNC
if HAVE_GPKNC
noinst_PROGRAMS = test-gknc
endif
endif

test_gknc_SOURCES = test-gknc.c
test_gknc_LDADD = $(top_builddir)/libgknc/libgknc.la $(GKNC_LIBS)

INCLUDES = $(GKNC_CFLAGS) -I$(top_srcdir)
if HAVE_GKNC
noinst_LTLIBRARIES = libgknc.la
endif

libgknc_la_SOURCES = gknc-preview.c gknc-preview.h

libgknc_la_LIBADD = $(top_builddir)/libknc/libknc.la $(GKNC_LIBS)

INCLUDES = $(GPKNC_CFLAGS) -I$(top_srcdir)

if HAVE_GPKNC
noinst_LTLIBRARIES = libgpknc.la
endif

libgpknc_la_SOURCES = gpknc-cntrl.h gpknc-cntrl.c
libgpknc_la_LIBADD = $(top_builddir)/libknc/libknc.la $(GPKNC_LIBS)

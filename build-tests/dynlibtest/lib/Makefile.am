# the library

EXTRA_DIST = dlt-lib.sym

lib_LTLIBRARIES = libdlt.la

libdlt_la_SOURCES = dlt-lib.c dlt-lib.h
libdlt_la_LDFLAGS = $(AM_LDFLAGS) \
	-export-dynamic -export-symbols $(srcdir)/dlt-lib.sym
libdlt_la_LIBADD = \
	$(LIBLTDL)
# The libtool docs describe these params, but they don't build.
#	"-dlopen" self \
#	"-dlopen" $(top_builddir)/lib/mod_a.la \
#	"-dlopen" $(top_builddir)/lib/mod_b.la
libdlt_la_CPPFLAGS = \
	$(LTDLINCL)	\
	$(AM_CPPFLAGS) $(CPPFLAGS)
libdlt_la_DEPENDENCIES = \
	$(LIBLTDL)

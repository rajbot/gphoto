bin_PROGRAMS = dlt-bin

dlt_bin_SOURCES = dlt-bin.c $(top_srcdir)/lib/dlt-lib.h
dlt_bin_CFLAGS = \
	$(AM_CPPFLAGS) $(CPPFLAGS)	\
	$(LTDLINCL) \
	-I$(top_srcdir)/lib
dlt_bin_LDFLAGS = $(AM_LDFLAGS) -export-dynamic
dlt_bin_LDADD = \
	$(LIBLTDL) \
	$(top_builddir)/lib/libdlt.la
# The libtool docs describe these params, but they don't build.
#	"-dlopen" self \
#	"-dlopen" $(top_builddir)/lib/foo.la \
#	"-dlopen" $(top_builddir)/lib/bar.la
dlt_bin_DEPENDENCIES = \
	$(LIBLTDL) \
	$(top_builddir)/lib/libdlt.la

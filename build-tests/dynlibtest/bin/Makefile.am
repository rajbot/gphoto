bin_PROGRAMS = dynlibtest-bin

dynlibtest_bin_SOURCES = dynlibtest-bin.c
dynlibtest_bin_CFLAGS = \
	$(AM_CFLAGS) $(CFLAGS)	\
	$(LTDLINCL)
dynlibtest_bin_LDFLAGS = $(AM_LDFLAGS) -export-dynamic
dynlibtest_bin_LDADD = \
	$(LIBLTDL)
# The libtool docs describe these params, but they don't build.
#	"-dlopen" self \
#	"-dlopen" $(top_builddir)/lib/foo.la \
#	"-dlopen" $(top_builddir)/lib/bar.la
dynlibtest_bin_DEPENDENCIES = \
	$(LIBLTDL) \
	$(top_builddir)/lib/foo.la \
	$(top_builddir)/lib/bar.la


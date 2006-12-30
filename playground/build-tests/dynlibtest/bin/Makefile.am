bin_PROGRAMS = dlt-bin

dlt_bin_SOURCES = dlt-bin.c $(top_srcdir)/lib/dlt-lib.h
dlt_bin_CPPFLAGS = \
	-I$(top_srcdir)/lib		\
	$(AM_CPPFLAGS) $(CPPFLAGS)
dlt_bin_LDFLAGS = $(AM_LDFLAGS) -export-dynamic
dlt_bin_LDADD = \
	$(top_builddir)/lib/libdlt.la
dlt_bin_DEPENDENCIES = \
	$(top_builddir)/lib/libdlt.la

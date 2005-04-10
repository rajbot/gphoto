testlib_LTLIBRARIES = foo.la bar.la

foo_la_SOURCES = dynlibtest-lib.c dynlibtest.h
foo_la_LDFLAGS = -module -no-undefined -avoid-version
foo_la_CFLAGS = \
	$(AM_CFLAGS) $(CFLAGS) \
	-DMODULE=foo

bar_la_SOURCES = dynlibtest-lib.c dynlibtest.h
bar_la_LDFLAGS = -module -no-undefined -avoid-version
bar_la_CFLAGS = \
	$(AM_CFLAGS) $(CFLAGS) \
	-DMODULE=bar

# End of Makefile.am.

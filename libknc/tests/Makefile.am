INCLUDES = -I$(top_srcdir) $(LIBKNCTEST_CFLAGS)

if HAVE_KNCTEST
noinst_PROGRAMS = test-knc
endif

test_knc_SOURCES = test-knc.c
test_knc_LDADD = $(top_builddir)/libknc/libknc.la $(LIBKNCTEST_LIBS) $(INTLLIBS)

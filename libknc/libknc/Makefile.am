INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/intl			\
	-DLOCALEDIR=\"$(datadir)/locale\"	\
	$(LIBKNC_CFLAGS)

#The library
lib_LTLIBRARIES = libknc.la
libknc_la_LIBADD = $(INTLLIBS) $(LIBKNC_LIBS)
libknc_la_LDFLAGS = -version-info @LIBKNC_VERSION_INFO@
libknc_la_SOURCES = knc.c knc-cntrl.c knc-transmit.h knc-cmd.c	\
	knc-cam-res.c knc-cntrl-res.c knc-utils.c

#The header files
libkncincludedir = $(includedir)/libknc
libkncinclude_HEADERS = knc.h knc-cntrl.h knc-cntrl-res.h knc-cam-res.h	\
	knc-cmd.h knc-utils.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libknc.pc

EXTRA_DIST = libknc.pc.in

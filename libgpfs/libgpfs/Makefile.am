INCLUDES = 					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/intl			\
	-DLOCALEDIR=\"$(datadir)/locale\"

# The library
lib_LTLIBRARIES = libgpfs.la
libgpfs_la_LIBADD = $(INTLLIBS)
libgpfs_la_LDFLAGS = -version-info @LIBGPFS_VERSION_INFO@
libgpfs_la_SOURCES = 	\
	gpfs-cache.c	\
	gpfs-err.c	\
	gpfs-err-type.c	\
	gpfs-file.c	\
	gpfs.c		\
	gpfs-if.c

# The header files
libgpfsincludedir = $(includedir)/libgpfs
libgpfsinclude_HEADERS =	\
	gpfs-cache.h		\
	gpfs-err.h		\
	gpfs-err-type.h		\
	gpfs-file.h		\
	gpfs-i18n.h		\
	gpfs.h			\
	gpfs-if.h		\
	gpfs-info.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgpfs.pc

EXTRA_DIST = libgpfs.pc.in

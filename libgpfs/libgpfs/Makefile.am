INCLUDES = 					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/intl			\
	-DLOCALEDIR=\"$(datadir)/locale\"

# The library
lib_LTLIBRARIES = libgpfs.la
libgpfs_la_LIBADD = $(INTLLIBS)
libgpfs_la_LDFLAGS = -version-info @LIBGPFS_VERSION_INFO@
libgpfs_la_SOURCES = 		\
	gpfs-cache.c		\
	gpfs-err.c		\
	gpfs-err-type.c		\
	gpfs-i18n.h		\
	gpfs-file.c		\
	gpfs.c			\
	gpfs-if.c		\
	gpfs-prop.c		\
	gpfs-val.c		\
	gpfs-val-type.c

# The header files
libgpfsincludedir = $(includedir)/libgpfs
libgpfsinclude_HEADERS =	\
	gpfs-cache.h		\
	gpfs-err.h		\
	gpfs-err-type.h		\
	gpfs-file.h		\
	gpfs.h			\
	gpfs-if.h		\
	gpfs-prop.h		\
	gpfs-val.h		\
	gpfs-val-type.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libgpfs.pc

EXTRA_DIST = libgpfs.pc.in

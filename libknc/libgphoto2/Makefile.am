libdir = $(GP_PATH)
if HAVE_GP
lib_LTLIBRARIES = libgphoto2_konica.la
endif

INCLUDES = -I$(top_srcdir) $(GP_CFLAGS)

libgphoto2_konica_la_SOURCES = library.c
libgphoto2_konica_la_LDFLAGS = -module -avoid-version
libgphoto2_konica_la_LIBADD  = $(top_builddir)/libknc/libknc.la $(GP_LIBS)

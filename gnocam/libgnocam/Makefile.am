INCLUDES =						\
	-I$(top_srcdir)					\
	$(LIBGNOCAM_CFLAGS)				\
	-DIMAGEDIR=\"$(prefix)/share/pixmaps\"		\
	-DGNOCAM_GLADE_DIR=\"$(GNOCAM_GLADE_DIR)\"	\
	-DGNOCAM_SRCDIR=\"$(top_srcdir)\"

noinst_LTLIBRARIES = libgnocam.la
libgnocam_la_SOURCES = gnocam-chooser.c gnocam-chooser.h
libgnocam_la_LDFLAGS = $(LIBGNOCAM_LIBS)

Gladedir = $(GNOCAM_GLADE_DIR)
Glade_DATA = gnocam.glade

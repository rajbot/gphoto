## Specify some includes and versioning
INCLUDES = -I../ $(VERSION_FLAGS) -I$(top_srcdir)/include
VERSION_FLAGS = -DLIBGPIO_VERSION=\"@LIBGPIO_VERSION@\"
CFLAGS = @CFLAGS@ @IOLIB_CFLAGS@ -g
LDFLAGS = @LDFLAGS@ -g

## Compile the IO library into a shared library
iolibdir = $(prefix)/lib/gpio
iolib_LTLIBRARIES = libgpio_ieee1394.la
libgpio_ieee1394_la_LDFLAGS = -version-info @LIBGPIO_VERSION_INFO@ -DHAVE_TERMIOS_H 
libgpio_ieee1394_la_SOURCES = linux-raw.c gpio-ieee1394.h

## Specify some includes and versioning
INCLUDES = -I../ $(VERSION_FLAGS) -I$(top_srcdir)/include
VERSION_FLAGS = -DLIBGPIO_VERSION=\"@LIBGPIO_VERSION@\"
CFLAGS = @CFLAGS@ @IOLIB_CFLAGS@ \
	-DIOLIBS=\"$(prefix)/lib/gpio\" \
	-g
LDFLAGS = @LDFLAGS@ -g -ldl

## Compile the IO library into a shared library
lib_LTLIBRARIES = libgpio.la
libgpio_la_LDFLAGS = -version-info @LIBGPIO_VERSION_INFO@
libgpio_la_SOURCES = 	gpio.c \
			library.c library.h \
			port.c

## Specify some includes and versioning
INCLUDES = -I../ $(VERSION_FLAGS) -I$(top_srcdir)/include
VERSION_FLAGS = -DLIBGPIO_VERSION=\"@LIBGPIO_VERSION@\"
CFLAGS = @CFLAGS@ @IOLIB_CFLAGS@ -g `libusb-config --cflags`
LDFLAGS = @LDFLAGS@ -g `libusb-config --libs`

## Compile the IO library into a shared library
iolibdir = $(prefix)/lib/gpio
iolib_LTLIBRARIES = libgpio_usb.la
libgpio_usb_la_LDFLAGS = -version-info @LIBGPIO_VERSION_INFO@
libgpio_usb_la_SOURCES = libusb.c

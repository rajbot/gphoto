## Process this file with automake to produce Makefile.in

INCLUDES = -I@srcdir@ $(VERSION_FLAGS)
VERSION_FLAGS = -DLIBGPIO_VERSION=\"@LIBGPIO_VERSION@\" -DHAVE_TERMIOS_H

if GPIO_USB
USB_SUPPORT = usb.c
else
USB_SUPPORT =
endif

lib_LTLIBRARIES = libgpio0.la
libgpio0_la_LDFLAGS = -version-info @LIBGPIO_VERSION_INFO@ -DHAVE_TERMIOS_H 
libgpio0_la_SOURCES = gpio.c gpio.h serial.c $(USB_SUPPORT)

gpioincdir = $(includedir)/gphoto
gpioinc_HEADERS = gpio.h

# DEPS = $(top_builddir)/libgpio.la
# LDADDS = $(top_builddir)/libgpio.la

EXTRA_DIST = AUTHORS ChangeLog COPYING.LIB NEWS README libgpio0.spec TODO

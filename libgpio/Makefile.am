## Process this file with automake to produce Makefile.in

INCLUDES = -I@srcdir@ $(VERSION_FLAGS)
VERSION_FLAGS = -DLIBGPIO_VERSION=\"@LIBGPIO_VERSION@\" -DHAVE_TERMIOS_H

bin_SCRIPTS = gpio-config
noinst_PROGRAMS = gpio_test

if GPIO_USB
USB_SUPPORT = gpio-usb.c
else
USB_SUPPORT =
endif

lib_LTLIBRARIES = libgpio.la
libgpio_la_LDFLAGS = -version-info @LIBGPIO_VERSION_INFO@ -DHAVE_TERMIOS_H 
libgpio_la_SOURCES = gpio.c gpio.h gpio-serial.c gpio-serial.h $(USB_SUPPORT)

gpio_test_SOURCES = gpio-test.c
gpio_test_LDADD = libgpio.la

gpioincdir = $(includedir)/gpio
gpioinc_HEADERS = gpio.h

# DEPS = $(top_builddir)/libgpio.la
# LDADDS = $(top_builddir)/libgpio.la

EXTRA_DIST = 		\
	AUTHORS \
	ChangeLog \
	COPYING.LIB \
	NEWS \
	README \
	libgpio.spec \
	gpio-config.in \
	gpioConf.sh.in \
	TODO

confexecdir=$(libdir)
confexec_DATA = gpioConf.sh

gpioConf.sh: gpioConf.sh.in Makefile
## Use sed and then mv to avoid problems if the user interrupts.
	sed -e 's?\@GPIO_LIBDIR\@?$(GPIO_LIBDIR)?g' \
	    -e 's?\@GPIO_INCLUDEDIR\@?$(GPIO_INCLUDEDIR)?g' \
	    -e 's?\@GPIO_LIBS\@?$(GPIO_LIBS)?g' \
	      < $(srcdir)/gpioConf.sh.in > gpioConf.tmp \
	  && mv gpioConf.tmp gpioConf.sh

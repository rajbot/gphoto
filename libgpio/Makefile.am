## Process this file with automake to produce Makefile.in

INCLUDES = -I@srcdir@ $(VERSION_FLAGS)
VERSION_FLAGS = -DLIBGPIO_VERSION=\"@LIBGPIO_VERSION@\" -DHAVE_TERMIOS_H

bin_SCRIPTS = gpio-config
noinst_PROGRAMS = gpio_test

if GPIO_USB
USB_HEADER = gpio-usb.h
USB_SOURCE = gpio-usb.c
else
USB_HEADER =
USB_SOURCE =
endif

if GPIO_IEEE1394
IEEE1394_HEADER = gpio-ieee1394.h
IEEE1394_SOURCE = gpio-ieee1394.c
else
IEEE1394_HEADER =
IEEE1394_SOURCE =
endif

if GPIO_NETWORK
NETWORK_HEADER = gpio-network.h
NETWORK_SOURCE = gpio-network.c
else
NETWORK_HEADER =
NETWORK_SOURCE =
endif


lib_LTLIBRARIES = libgpio.la
libgpio_la_LDFLAGS = -version-info @LIBGPIO_VERSION_INFO@ -DHAVE_TERMIOS_H 
libgpio_la_SOURCES = 	gpio.c gpio.h \
			gpio-serial.c gpio-serial.h \
			gpio-parallel.c gpio-parallel.h \
			$(NETWORK_HEADER) $(NETWORK_SOURCE) \
			$(IEEE1394_HEADER) $(IEEE1394_SOURCE) \
			$(USB_HEADER) $(USB_SOURCE)

gpio_test_SOURCES = gpio-test.c
gpio_test_LDADD = libgpio.la

gpioincdir = $(includedir)/gpio
gpioinc_HEADERS = gpio.h gpio-serial.h gpio-parallel.h $(NETWORK_HEADER) $(IEEE1394_HEADER) $(USB_HEADER) 

# DEPS = $(top_builddir)/libgpio.la
# LDADDS = $(top_builddir)/libgpio.la

confexecdir=$(libdir)
confexec_DATA = gpioConf.sh

CLEANFILES=gpioConf.sh

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

gpioConf.sh: gpioConf.sh.in Makefile
## Use sed and then mv to avoid problems if the user interrupts.
	sed -e 's?\@GPIO_LIBDIR\@?$(GPIO_LIBDIR)?g' \
	    -e 's?\@GPIO_INCLUDEDIR\@?$(GPIO_INCLUDEDIR)?g' \
	    -e 's?\@GPIO_LIBS\@?$(GPIO_LIBS)?g' \
	      < $(srcdir)/gpioConf.sh.in > gpioConf.tmp \
	  && mv gpioConf.tmp gpioConf.sh

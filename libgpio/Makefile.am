## Process this file with automake to produce Makefile.in

## Specify some includes and versioning
INCLUDES = -I@srcdir@ $(VERSION_FLAGS)
VERSION_FLAGS = -DLIBGPIO_VERSION=\"@LIBGPIO_VERSION@\" -DHAVE_TERMIOS_H

## Executable files
bin_SCRIPTS = gpio-config

SUBDIRS = libgpio include @IOLIB_SUBDIRS@

## GNOME configuration script
confexecdir=$(libdir)
confexec_DATA = gpioConf.sh

## Extra files 
CLEANFILES=gpioConf.sh
EXTRA_DIST = 		\
	AUTHORS \
	ChangeLog \
	COPYING.LIB \
	NEWS \
	README \
	libgpio.spec \
	gpio-config.in \
	gpioConf.sh.in 

gpioConf.sh: gpioConf.sh.in Makefile
## Use sed and then mv to avoid problems if the user interrupts.
	sed -e 's?\@GPIO_LIBDIR\@?$(GPIO_LIBDIR)?g' \
	    -e 's?\@GPIO_INCLUDEDIR\@?$(GPIO_INCLUDEDIR)?g' \
	    -e 's?\@GPIO_LIBS\@?$(GPIO_LIBS)?g' \
	    -e 's?\@LIBGPIO_VERSION\@?$(LIBGPIO_VERSION)?g' \
	      < $(srcdir)/gpioConf.sh.in > gpioConf.tmp \
	  && mv gpioConf.tmp gpioConf.sh

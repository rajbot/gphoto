drivers_LTLIBRARIES = libgphoto_philips.la

CFLAGS = @GTK_CFLAGS@ -DHAVE_TERMIOS_H=1

libgphoto_philips_la_SOURCES = philips_io.c gphoto_philips.c \
	gphoto_philips_config.c \
	crctab.h debug.h philips_io.h philips.h

libgphoto_philips_la_LDFLAGS = -version-info 0:0:0

drivers_LTLIBRARIES = libgphoto_photopc.la

CFLAGS = @GTK_CFLAGS@ -DUNIX -DINT16=short -DHAVE_CFMAKERAW -DHAVE_STRFTIME \
		-DHAVE_PRIOCTL -DHAVE_NANOSLEEP

libgphoto_photopc_la_SOURCES = eph_read.c eph_iob.c eph_err.c eph_cmd.c \
	eph_io.c eph_open.c olympus.c \
	eph_io.h eph_priv.h

libgphoto_photopc_la_LDFLAGS = -version-info 0:0:0

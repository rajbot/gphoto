drivers_LTLIBRARIES = libgphoto_quickcam.la

CFLAGS = @CFLAGS@ @GTK_CFLAGS@ -O

libgphoto_quickcam_la_SOURCES = qcam_gphoto.c qcam-Linux.c\
	qcam_drv.c edge.c exposure.c histogram.c qcam_gfx.c scale.c \
	qcam.h qcam_gfx.h

EXTRA_DIST = qcam-Linux.c qcam-Linux.h qcam-FreeBSD.c qcam-FreeBSD.h

libgphoto_quickcam_la_LDFLAGS = -version-info 0:0:0

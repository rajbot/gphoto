bin_PROGRAMS = gphoto2-ftp

INCLUDES =					\
	-I$(top_srcdir)				\
	-I$(top_srcdir)/intl			\
	-I$(top_srcdir)/gphoto2-ftp		\
	$(GP_CFLAGS)

gphoto2_ftp_SOURCES =					\
	gphoto2-ftp.c					\
	gphoto2-ftp-conn.c gphoto2-ftp-conn.h		\
	gphoto2-ftp-cwd.c  gphoto2-ftp-cwd.h		\
	gphoto2-ftp-list.c gphoto2-ftp-list.h		\
	gphoto2-ftp-params.h				\
	gphoto2-ftp-pass.c gphoto2-ftp-pass.h		\
	gphoto2-ftp-pasv.c gphoto2-ftp-pasv.h		\
	gphoto2-ftp-port.c gphoto2-ftp-port.h		\
	gphoto2-ftp-retr.c gphoto2-ftp-retr.h		\
	gphoto2-ftp-type.c gphoto2-ftp-type.h		\
	gphoto2-ftp-user.c gphoto2-ftp-user.h

gphoto2_ftp_LDADD = $(POPT_LIBS) $(GP_LIBS) $(PTHREAD_LIBS)

modulesdir = $(libdir)/gnome-vfs-2.0/modules
modules_LTLIBRARIES=libcamera.la

CORBA_GENERATED = GNOME_C-common.c GNOME_C-stubs.c GNOME_C.h
$(CORBA_GENERATED): $(top_srcdir)/idl/GNOME_C.idl
	$(ORBIT_IDL) --noskels \
		-I`$(PKG_CONFIG) --variable=idldir libbonobo-2.0` \
		-I`$(PKG_CONFIG) --variable=idldir bonobo-activation-2.0` \
		$(top_srcdir)/idl/GNOME_C.idl

libcamera_la_SOURCES =			\
	$(CORBA_GENERATED)		\
	camera-method.c			\
	camera-dir.h camera-dir.c	\
	camera-file.h camera-file.c	\
	camera-utils.h camera-utils.c

INCLUDES = -DG_LOG_DOMAIN=\"libcamera\" $(VFS_CFLAGS)
libcamera_la_LIBADD = $(VFS_LIBS)
libcamera_la_LDFLAGS = -export_dynamic -avoid-version

modulesconfdir = $(sysconfdir)/gnome-vfs-2.0/modules
modulesconf_DATA=camera-methods.conf

EXTRA_DIST=camera-methods.conf
CLEANFILES=$(CORBA_GENERATED)

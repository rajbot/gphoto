## Process this file with automake to create a GNU Makefile

SUBDIRS = icons

bin_PROGRAMS = gphoto

CFLAGS = @CFLAGS@ -I$(srcdir)/icons @GTK_CFLAGS@ @GDK_IMLIB_CFLAGS@

gphoto_LDADD = ../konica/libgphoto_konica_qm100.la \
	../konica_qmxxx/libgphoto_konica_qmxxx.la \
	../canon/libgphoto_canon.la \
	../casio/libgphoto_casio_qv.la \
	../nikon/libgphoto_coolpix600.la \
	../philips/libgphoto_philips.la \
	../fuji/libgphoto_fuji.la \
	../photopc/libgphoto_photopc.la \
	../kodak/libgphoto_kodak_dc2x.la \
	../kodak_generic/libgphoto_kodak_generic.la \
	../minolta/libgphoto_dimage_v.la \
	../mustek/libgphoto_mustek_mdc800.la \
	../ricoh/libgphoto_ricoh300z.la \
	../sony/libgphoto_sony_dscf1.la \
	../sony/dscf55/libgphoto_sony_dscf55.la \
	../directory/libgphoto_dir.la \
	@GTK_LIBS@ @GDK_IMLIB_LIBS@ -ljpeg

gphoto_SOURCES =  main.c commandline.c live.c gallery.c menu.c \
	toolbar.c callbacks.c util.c developer_dialog.c img_edit.c gphoto.c \
	main.h commandline.h live.h gallery.h menu.h toolbar.h callbacks.h \
	util.h cameras.h gphoto.h developer.h developer_dialog.h

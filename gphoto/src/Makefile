# $Id$
#
# $Log$
# Revision 1.4  1999/05/31 18:51:52  ole
# 1999-05-31  Ole Aamot  <oleaa@ifi.uio.no>
#
# 	* casio/Makefile: clean now removes config.log and config.stats
# 	* removed the gphoto-bug script.  Use the "debbugs" module
#
# Revision 1.3  1999/05/31 18:32:26  ole
# 1999-05-31  Ole Aamot  <oleaa@ifi.uio.no>
#
# 	* modified the "gnome-bugs" script, and added gphoto-bug
# 	* added more supported models to camera.h
# 	* removed some cruft from the FAQ
#
# Revision 1.2  1999/05/28 20:24:07  ole
# Gallery themes now installs correctly.
# Changed the URL for Help - FAQs entry.
#
# Revision 1.1.1.1  1999/05/27 18:32:06  scottf
# gPhoto- digital camera utility
#
# Revision 1.38  1999/05/10 18:03:04  mmartin
# Added the color adjustment dialog ripped (with permission) from Electric Eyes,
# activated from the toolbar with a new button.
#
# Revision 1.37  1999/05/09 21:20:11  ole
# Moved the "gphoto" wrapper to "gphoto.sh".  Modified Makefile to compile
# and install "gphoto".
#
# Revision 1.36  1999/05/09 15:23:38  ole
# AUTHORS for developers, CREDITS for beta-testers. :-)
#
# Revision 1.35  1999/05/09 05:47:27  gdr
# Changed hard coded path (/usr/doc) to Makefile variable (DOCDIR)
#
# Revision 1.34  1999/05/09 00:08:49  mmartin
# Added an "rpath" to the linking step with the hope of removing the wrapper
#
# Revision 1.33  1999/05/08 22:42:52  gdr
# Fixed typo
#
# Revision 1.32  1999/05/08 18:08:24  gdr
# Modified to hopefully make more portable
#
# Revision 1.31  1999/05/08 04:10:14  scottf
# added -I/usr/lib/glib/include to paths in some Makefiles
# and fixed a path in developer_dialog.c (/usr/doc/gphoto-0.3).
# finally, added "image_type" to the Image struct
#
# Revision 1.30  1999/05/06 15:08:28  ole
# Small adjustments.
#
# Revision 1.29  1999/05/06 15:00:43  ole
# Added /usr/doc/gphoto-0.3 in (un)install:
#
# This Makefile is totally rehashed from the old one. Please keep it tidy
# It will be replaced with a ./configure but that's a good reason to keep
# it clean. Its difficult to produce a clean script if the template it is
# working toward is not minimal. 

# I've removed the 'gtk-configs' etc since they produce the same outputs.
# configure will need INCDIRS and things so its best to head in that
# direction now. - Phill

PREFIX		=	/usr/local
BINDIR		=	$(PREFIX)/bin
DOCDIR		=	$(PREFIX)/doc
MANDIR		=	$(PREFIX)/man
GPHOTODIR	=	$(PREFIX)/share/gphoto

CARGS = -g -D_GNU_SOURCE -Wall -m486 -DDOCDIR="\"$(DOCDIR)\""

GTK_CFLAGS = $(shell gtk-config --cflags)
GTK_LIBS = $(shell gtk-config --libs)

INCDIR = $(GTK_CFLAGS) -I./icons/

CAMLINKDIR = -L../konica \
	-L../philips \
	-L../fuji \
	-L../directory \
	-L../photopc \
	-L../ricoh \
	-L../kodak \
	-L../sony \
	-L../casio
LDARGS = -lgdk_imlib $(GTK_LIBS) -ljpeg -lgif
CAMLDARGS = -Wl,-rpath,$(GPHOTODIR)/drivers\
	-lgphoto_konica_qm100 \
	-lgphoto_casio_qv \
	-lgphoto_philips \
	-lgphoto_fuji \
	-lgphoto_dir \
	-lgphoto_photopc \
	-lgphoto_kodak_dc2x \
	-lgphoto_ricoh300z \
	-lgphoto_sony_dscf1 \

OBJS =  main.o \
	commandline.o \
	live.o \
	gallery.o \
	menu.o \
	toolbar.o \
	developer_dialog.o \
	util.o \
	callbacks.o \
	img_edit.o

SH_LIBS = ../konica/libgphoto_konica_qm100.so \
	../philips/libgphoto_philips.so \
	../casio/libgphoto_casio_qv.so \
	../fuji/libgphoto_fuji.so \
	../photopc/libgphoto_photopc.so \
	../kodak/libgphoto_kodak_dc2x.so \
	../ricoh/libgphoto_ricoh300z.so \
	../sony/libgphoto_sony_dscf1.so \
	../directory/libgphoto_dir.so

.c.o:
	gcc ${CFLAGS} ${CARGS} -c $< ${INCDIR}

gphoto:	${OBJS} ${LIBS} ${SH_LIBS}
	gcc ${CFLAGS} ${CARGS} -o gphoto ${OBJS} ${LIBS} ${LINKDIR} ${CAMLDARGS} ${CAMLINKDIR} ${LDARGS}

install:
	cp gphoto $(BINDIR)
	cp ../man/gphoto.1 $(MANDIR)/man1
	mkdir -p $(GPHOTODIR)
	mkdir -p $(GPHOTODIR)/drivers
	mkdir -p $(DOCDIR)/gphoto-0.3
	cp ../AUTHORS $(DOCDIR)/gphoto-0.3/
	cp ../CREDITS $(DOCDIR)/gphoto-0.3/
	cp ../README $(DOCDIR)/gphoto-0.3/
	cp ../MANUAL $(DOCDIR)/gphoto-0.3/
	cp ../FAQ $(DOCDIR)/gphoto-0.3/
	cp ../NEWS $(DOCDIR)/gphoto-0.3/
	cp -R ../gallery $(GPHOTODIR)
	cp ../directory/libgphoto_dir.so $(GPHOTODIR)/drivers
	cp ../konica/libgphoto_konica_qm100.so $(GPHOTODIR)/drivers
	cp ../casio/libgphoto_casio_qv.so $(GPHOTODIR)/drivers
	cp ../philips/libgphoto_philips.so $(GPHOTODIR)/drivers
	cp ../fuji/libgphoto_fuji.so $(GPHOTODIR)/drivers
	cp ../photopc/libgphoto_photopc.so $(GPHOTODIR)/drivers
	cp ../kodak/libgphoto_kodak_dc2x.so $(GPHOTODIR)/drivers
	cp ../ricoh/libgphoto_ricoh300z.so $(GPHOTODIR)/drivers
	cp ../sony/libgphoto_sony_dscf1.so $(GPHOTODIR)/drivers

uninstall:
	rm -rf ~/.gphoto/
	rm -f $(BINDIR)/gphoto
	rm -f $(BINDIR)/gphoto.bin
	rm -f $(MANDIR)/man1/gphoto.1
	rm -rf $(GPHOTODIR)
	rm -rf $(DOCDIR)/gphoto-0.3/

../ricoh/libgphoto_ricoh300z.so:
	make -C ../ricoh CFLAGS="$(CARGS) $(INCDIR)" libgphoto_ricoh300z.so

../kodak/libgphoto_kodak_dc2x.so:
	make -C ../kodak CFLAGS="$(CARGS) $(INCDIR)" libgphoto_kodak_dc2x.so

../photopc/libgphoto_photopc.so:
	make -C ../photopc CFLAGS="$(CARGS) $(INCDIR)" libgphoto_photopc.so

../fuji/libgphoto_fuji.so:
	make -C ../fuji CFLAGS="$(CARGS) $(INCDIR)" libgphoto_fuji.so

../konica/libgphoto_konica_qm100.so:
	make -C ../konica CFLAGS="$(CARGS) $(INCDIR)" libgphoto_konica_qm100.so

../casio/libgphoto_casio_qv.so:
	make -C ../casio CFLAGS="$(CARGS) $(INCDIR)" libgphoto_casio_qv.so

../philips/libgphoto_philips.so:
	make -C ../philips CFLAGS="$(CARGS) $(INCDIR)" libgphoto_philips.so

../sony/libgphoto_sony_dscf1.so:
	make -C ../sony CFLAGS="$(CARGS) $(INCDIR)" libgphoto_sony_dscf1.so

../directory/libgphoto_dir.so:
	make -C ../directory CFLAGS="$(CARGS) $(INCDIR)" libgphoto_dir.so

dist-clean:	
	make -C ../photopc clean
	make -C ../ricoh clean
	make -C ../kodak clean
	make -C ../fuji clean
	make -C ../philips clean
	make -C ../konica clean
	make -C ../casio clean
	make -C ../sony clean
	make -C ../directory clean 
	rm -f *.o gphoto core

distclean: dist-clean

clean:
	rm -f *.o gphoto core

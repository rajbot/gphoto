# This Makefile is totally rehashed from the old one. Please keep it tidy
# It will be replaced with a ./configure but that's a good reason to keep
# it clean. Its difficult to produce a clean script if the template it is
# working toward is not minimal. 

# I've removed the 'gtk-configs' etc since they produce the same outputs.
# configure will need INCDIRS and things so its best to head in that
# direction now. - Phill

CARGS = -g -D_GNU_SOURCE -Wall -m486
INCDIR = -I/usr/include -I/usr/X11R6/include -I/usr/X11R6/include/X11 -I/usr/lib/glib/include -I../src/icons/
LINKDIR = -L/usr/lib -L/usr/X11R6/lib
CAMLINKDIR = -L../konica -L../philips -L../fuji -L../gphoto_dir -L../photopc -L../ricoh_300 -L../kodak
LDARGS = -lgdk_imlib -lgtk -lgdk -rdynamic -lgmodule -lglib -ldl -lXext \
		-lX11 -lm -ljpeg -lgif
CAMLDARGS =
#-lgphoto_fuji  -lgphoto_dir  -lgphoto_ricoh300z   -lgphoto_photopc  -lgphoto_konica_qm100  -lgphoto_philips -lgphoto_kodak_dc2x
OBJS =  main.o \
	commandline.o \
	live.o \
	gallery.o \
	menu.o \
	toolbar.o \
	callbacks.o 

SH_LIBS = ../konica/libgphoto_konica_qm100.so \
	../philips/libgphoto_philips.so \
	../fuji/libgphoto_fuji.so \
	../photopc/libgphoto_photopc.so \
	../kodak/libgphoto_kodak_dc2x.so \
	../ricoh_300/libgphoto_ricoh300z.so \
	../gphoto_dir/libgphoto_dir.so

.c.o:
	gcc ${CFLAGS} ${CARGS} -c $< ${INCDIR}

gphoto:	${OBJS} ${LIBS} ${SH_LIBS} cameras.h main.h
	gcc ${CFLAGS} ${CARGS} -o gphoto ${OBJS} ${LIBS} ${LINKDIR} ${CAMLDARGS} ${CAMLINKDIR} ${LDARGS}

install:
	cp gphoto.bin /usr/local/bin
	cp gphoto /usr/local/bin
	cp ../man/gphoto.1 /usr/local/man/man1
	mkdir -p /usr/share/gphoto
	mkdir -p /usr/share/gphoto/drivers
	cp -R ../gallery /usr/share/gphoto
	cp ../gphoto_dir/libgphoto_dir.so /usr/share/gphoto/drivers
	cp ../konica/libgphoto_konica_qm100.so /usr/share/gphoto/drivers
	cp ../philips/libgphoto_philips.so /usr/share/gphoto/drivers
	cp ../fuji/libgphoto_fuji.so /usr/share/gphoto/drivers
	cp ../photopc/libgphoto_photopc.so /usr/share/gphoto/drivers
	cp ../kodak/libgphoto_kodak_dc2x.so /usr/share/gphoto/drivers
	cp ../ricoh_300/libgphoto_ricoh300z.so /usr/share/gphoto/drivers

uninstall:
	rm -f /usr/local/bin/gphoto
	rm -f /usr/local/man/man1/gphoto.1

../ricoh_300/libgphoto_ricoh300z.so:
	make -C ../ricoh_300 libgphoto_ricoh300z.so

../kodak/libgphoto_kodak_dc2x.so:
	make -C ../kodak libgphoto_kodak_dc2x.so

../photopc/libgphoto_photopc.so:
	make -C ../photopc libgphoto_photopc.so

../fuji/libgphoto_fuji.so:
	make -C ../fuji libgphoto_fuji.so

../konica/libgphoto_konica_qm100.so:
	make -C ../konica libgphoto_konica_qm100.so

../philips/libgphoto_philips.so:
	make -C ../philips libgphoto_philips.so

../gphoto_dir/libgphoto_dir.so:
	make -C ../gphoto_dir libgphoto_dir.so

dist-clean:	
	make -C ../photopc clean
	make -C ../ricoh_300 clean
	make -C ../kodak clean
	make -C ../fuji clean
	make -C ../philips clean
	make -C ../konica clean
	make -C ../gphoto_dir clean 
	rm -f *.o gphoto.bin core

clean:
	rm -f *.o gphoto.bin core

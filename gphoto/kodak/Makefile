CFLAGS=-DDC25=1 -I/usr/lib/glib/include
.c.o:
	gcc ${CFLAGS} -c -fPIC $<

OBJS= \
	kodak_dc2x.o \
	change_res.o \
	comet_to_ppm.o \
	convert_pic.o \
	erase.o \
	get_info.o \
	get_pic.o \
	get_thumb.o \
	hash_mark.o \
	init_dc20.o \
	pics_to_file.o \
	pixmaps.o \
	read_data.o \
	send_pck.o \
	session.o \
	shoot.o \
	thumbs_to_file.o

libgphoto_kodak_dc2x.so: ${OBJS}
	gcc -shared -Wl,-soname,libgphoto_kodak_dc2x.so -o libgphoto_kodak_dc2x.so ${OBJS}

libkodak_dc2x.a: ${OBJS}
	ar -rc libkodak_dc2x.a ${OBJS}

clean:
	rm -f *.o libkodak_dc2x.a libgphoto_kodak_dc2x.so


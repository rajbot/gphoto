CFLAGS = -I/usr/lib/glib/include
OBJS = fujilib.o fuji_exif.o exif.o

libgphoto_fuji.so: fujilib.o fuji_exif.o exif.o
	gcc -shared -Wl,-soname,libgphoto_fuji.so -o libgphoto_fuji.so ${OBJS}

libfuji.a: fujilib.o fuji_exif.o
	ar -rc libfuji.a fujilib.o fuji_exif.o exif.o

.c.o:
		gcc ${CFLAGS} -g -c -fPIC $<

exifdump: exifdump.c exif.o exif.h
	gcc -g -o exifdump exifdump.c exif.o

clean:
	rm -f libfuji.a fujilib.o fuji_exif.o exif.o libgphoto_fuji.so

OPTS = -g -DHAVE_TERMIOS_H=1
CFLAGS = -I/usr/lib/glib/include
OBJS = ricoh_300_io.o ricoh_300z.o

libricoh300z.a: ${OBJS}
	ar -rc libricoh300z.a ${OBJS}

libgphoto_ricoh300z.so: ${OBJS}
	gcc -shared -Wl,-soname,libgphoto_ricoh300z.so -o libgphoto_ricoh300z.so ${OBJS}

.c.o:
	gcc ${CFLAGS} ${OPTS} -c -fPIC $<

clean:
	rm -f *.o libgphoto_ricoh300z.so libricoh300z.a

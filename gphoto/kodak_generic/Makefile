CFLAGS=-DHAVE_TERMIOS_H
.c.o:
	gcc ${CFLAGS} -c -fPIC $<

OBJS= io.o \
      kodak_generic.o

libgphoto_kodak_generic.so: ${OBJS}
	gcc -shared -Wl,-soname,libgphoto_kodak_generic.so -o libgphoto_kodak_generic.so ${OBJS}

libkodak_generic.a: ${OBJS}
	ar -rc libkodak_generic.a ${OBJS}

clean:
	rm -f *.o libkodak_generic.a libgphoto_kodak_generic.so


CFLAGS = -g -I/usr/lib/glib/include

OBJS = directory.o 

.c.o:	
	gcc ${CFLAGS} -c -fPIC $<

libgphoto_dir.so: ${OBJS}
	gcc -shared -Wl,-soname,libgphoto_dir.so -o libgphoto_dir.so ${OBJS}

libgphoto_dir.a: ${OBJS}
	ar -rc libgphoto_dir.a ${OBJS}

clean:
	rm -f *~ *.o core libgphoto_dir.so libgphoto_dir.a

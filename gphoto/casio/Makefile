XINCDIRS =  -I/usr/X11R6/include -I/usr/X11R6/include/X11
INCDIRS = -I../src $(XINCDIRS) -I./icons/
GLIB_CFLAGS = $(shell glib-config --cflags)
GTK_CFLAGS = $(shell gtk-config --cflags)
CFLAGS = -g -Wall $(INCDIRS) $(GTK_CFLAGS) $(GLIB_CFLAGS)

CSOURCE = getuint.c ppm.c jpeg.c printMsg.c sdComm.c \
	  configure.c config_handler.c qv_io.c command.c casio_qv.c

OBJS = $(CSOURCE:%.c=%.o)

.c.o:	
	gcc $(CFLAGS) -c -fPIC $<

libgphoto_casio_qv.so: $(OBJS)
	gcc -g -shared -Wl,-soname,libgphoto_casio_qv.so -o libgphoto_casio_qv.so $(OBJS)

casio_qv.a: $(OBJS)
	ar -rc casio_qv.a $(OBJS)

clean:
	rm -f *~ *.o core casio_qv.a libgphoto_casio_qv.so \
		configure config.h config.h.in config.cache

config.h.in:
	autoheader

configure: configure.in
	autoconf

config.h: config.h.in configure
	./configure

depend: $(CSOURCE)
	gcc -MM $(CFLAGS) -fPIC $(CSOURCE) > Make.depend

include Make.depend

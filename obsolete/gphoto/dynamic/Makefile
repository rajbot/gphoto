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
CAMLINKDIR = -L../konica
LDARGS = -lgdk_imlib -lgtk -lgdk -rdynamic -lgmodule -lglib -ldl -lXext -lX11 -lm
CAMLDARGS = -lkonica_qm100

OBJS =  main.o \
	commandline.o \
	live.o \
	gallery.o \
	menu.o \
	toolbar.o \
	callbacks.o

#	directory.o

.c.o:
	gcc ${CFLAGS} ${CARGS} -c $< ${INCDIR}

gphoto:	${OBJS}
	gcc ${CFLAGS} ${CARGS} -o gphoto ${OBJS} ${LDARGS} ${LINKDIR} ${CAMLDARGS} ${CAMLINKDIR}

install:
	cp gphoto /usr/local/bin
	cp ../man/gphoto.1 /usr/local/man/man1
	mkdir -p /usr/share/gphoto
	cp -R ../gallery /usr/share/gphoto

uninstall:
	rm -f /usr/local/bin/gphoto
	rm -f /usr/local/man/man1/gphoto.1

libkonica:
	make -C ../konica

clean:	
	make -C ../konica clean
	rm -f *.o gphoto core



# main.o: main.c main.h callbacks.h directory.h gallery.h menu.h toolbar.h
# cameras.c commandline.h

#directory.o: main.h directory.h directory.c
#callbacks.o: main.h callbacks.h callbacks.c
#toolbar.o: main.h toolbar.c toolbar.h
#menu.o: main.h menu.c menu.h
#live.o: main.h live.c live.h
#gallery.o: main.h gallery.c gallery.h
#commandline.o: main.h commandline.c commandline.h


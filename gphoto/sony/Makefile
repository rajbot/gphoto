# Generated automatically from Makefile.in by configure.

srcdir	= .

CC	= gcc
LDCC	= $(CC)
CFLAGS	= -g -O -I/usr/lib/glib/include -I../src/icons
LIBS	= 
INSTALL	= /usr/bin/install -c
INSTALL_PROGRAM	= ${INSTALL}
INSTALL_DATA	= ${INSTALL} -m 644
#bindir	= /usr/local/bin

SRCS1	= tty.c command.c getuint.c chotplay.c
LIBOBJS	= tty.o command.o getuint.o chotplay.o dsc_f1.o
#PROG1	= chotplay

#SRCS2	= tty.c command.c getuint.c chotrec.c
#OBJS2	= tty.o command.o getuint.o chotrec.o
#PROG2	= chotrec

.c.o:
	$(CC) $(CFLAGS) $(DEFS) -c -fPIC $<

#all:	$(PROG1) $(PROG2)
all: libgphoto_sony_dscf1.so

libgphoto_sony_dscf1.so: ${LIBOBJS}
	gcc -shared -W1,-soname,libgphoto_sony_dscf1.so -o libgphoto_sony_dscf1.so ${LIBOBJS}

$(PROG1): $(OBJS1)
	$(LDCC) -o $(PROG1) $(CFLAGS) $(OBJS1) $(LIBS)

$(PROG2): $(OBJS2)
	$(LDCC) -o $(PROG2) $(CFLAGS) $(OBJS2) $(LIBS)

clean:
	rm -f $(PROG1) $(LIBOBJS) $(PROG2) $(OBJS2) libgphoto_sony_dscf1.so

install:	all
	cp -f libgphoto_sony_dscf1.so /usr/share/gphoto/drivers


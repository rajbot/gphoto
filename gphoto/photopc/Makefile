#	$Id$

#	Copyright (c) 1997,1998 Eugene G. Crosser
#	Copyright (c) 1998 Bruce D. Lightner (DOS/Windows support)
#
#	You may distribute and/or use for any purpose modified or unmodified
#	copies of this software if you preserve the copyright notice above.
#
#	THIS SOFTWARE IS PROVIDED AS IS AND COME WITH NO WARRANTY OF ANY
#	KIND, EITHER EXPRESSED OR IMPLIED.  IN NO EVENT WILL THE
#	COPYRIGHT HOLDER BE LIABLE FOR ANY DAMAGES RESULTING FROM THE
#	USE OF THIS SOFTWARE.

#	$Log$
#	Revision 1.1  1999/05/27 18:32:05  scottf
#	Initial revision
#
#	Revision 1.6  1999/05/09 00:37:40  scottf
#	added misc changes to the gphoto_dir, olympus, and sony_dsc1 files.
#	also removed some old code from callbacks, and fixed the bath-save bug
#	that only saved the full-size images as the -thumbnail files.
#	
#	Revision 1.5  1999/05/08 04:10:14  scottf
#	added -I/usr/lib/glib/include to paths in some Makefiles
#	and fixed a path in developer_dialog.c (/usr/doc/gphoto-0.3).
#	finally, added "image_type" to the Image struct
#	
#	Revision 1.4  1999/05/02 02:41:13  scottf
#	many changes. updated the sony_dsc lib for the Image* struct.
#	
#	Revision 1.3  1999/03/21 03:11:16  phill
#	Dynamic updates for all libs.
#	
#	Phill
#	
#	Revision 1.2  1999/03/18 06:30:53  phill
#	*** empty log message ***
#	
#	Revision 1.1.1.1  1999/01/07 15:04:02  del
#	Imported 0.2 sources
#	
#	Revision 2.9  1998/10/18 18:59:16  crosser
#	fix a glitch
#
#	Revision 2.8  1998/10/18 13:18:27  crosser
#	Put RCS logs and I.D. into the source
#
#	Revision 2.7  1998/08/01 12:30:50  crosser
#	Create dirs during install
#	
#	Revision 2.6  1998/02/27 08:38:06  crosser
#	correct OPTS for other OS-es
#	
#	Revision 2.5  1998/02/16 06:11:22  lightner
#	Add dependency for revision.h
#	
#	Revision 2.4  1998/02/08 19:58:38  crosser
#	Support low memory: chunked saving etc.
#	
#	Revision 2.3  1998/01/27 21:52:23  crosser
#	add NANOSLEEP
#	
#	Revision 2.2  1998/01/18 02:16:45  crosser
#	DOS support
#	HAVE_NANOSLEEP
#	
#	Revision 2.1  1998/01/04 13:55:57  crosser
#	add HAVE_PRIOCTL
#	
#	Revision 2.0  1998/01/02 19:20:11  crosser
#	Added support for Win32
#	
#	Revision 1.2  1997/12/24 00:19:13  crosser
#	add HAVE_STRFTIME option
#	
#	Revision 1.1  1997/08/17 08:59:54  crosser
#	Initial revision
#

# -DUNIX
# Use UNIX specific I/O and other things.

# -DDOS
# Use DOS specific I/O and other things.  Do not define here, use
# Makefile.dos file instead.

# -DMSWINDOWS
# Use Windows specific I/O and other things.  Do not define here, use
# Makefile.win file instead.

# -DLOWMEMORY
# Try to work even if the whole image dill not fit in addressable
# memory (e.g DOS or XENIX/286)

# -DINT16=<type-for-16bit-integer>
# Here you need to specify which integer type correspond to 16bit word.
# On Intel x86 and SPARC it is `short' and I think it is the same on
# most reasonable architectures.  But if you have anything really exotic,
# watch out!

# -DHAVE_CFMAKERAW
# Some modern unices have cfmakeraw() function.  If you are not sure,
# do not define this, it will work anyway.

# -DHAVE_SELECT_H
# On some unices, you need to #include <sys/select.h> to use select()
# system call.  On others, you don't and you cannot.  Look if there is
# a file "/usr/include/sys/select.h" and define accordingly.

# -DHAVE_STRFTIME
# use strftime(3) to make file names with format given in -f option

# -DHAVE_PRIOCTL
# use sched_setsheduler() to obtain realtime proprity if possible.
# Will only work if run by root or setuid root.

# -DHAVE_NANOSLEEP
# use nanlosleep() instead of usleep() for short delays.  This may make
# short delays more accurate on some systems (e.g Linux with libc-4) if
# used togrther with HAVE_PRIOCTL.

# This one is good for Linux:
OPTS	= -DUNIX -DINT16=short -DHAVE_CFMAKERAW -DHAVE_STRFTIME \
		-DHAVE_PRIOCTL -DHAVE_NANOSLEEP

# This one is good for Solaris 2.x:
#OPTS	= -DUNIX -DINT16=short -DHAVE_SELECT_H -DHAVE_STRFTIME

# This one is good for BSDi 3.x, NetBSD and FreeBSD
#OPTS	= -DUNIX -DINT16=short -DHAVE_SELECT_H -DHAVE_STRFTIME -DHAVE_CFMAKERAW


BINDEST	= /usr/local/bin
LIBDEST	= /usr/local/lib
INCDEST	= /usr/local/include
MANDEST	= /usr/local/man
MANEXT	= 1

CC	= gcc
CFLAGS	= -O2 -Wall -I/usr/lib/glib/include
LD	= gcc
LDFLAGS	=
AR	= ar
RANLIB	= ranlib
INSTALL	= install -c

##### No need to touch anything below this line #####

LIBOBJS	= eph_read.o eph_iob.o eph_err.o eph_cmd.o eph_io.o eph_open.o olympus.o

.c.o:
	${CC} ${CFLAGS} ${OPTS} -c -fPIC $<

all:	libgphoto_photopc.so

libgphoto_photopc.so: ${LIBOBJS}
	gcc -shared -Wl,-soname,libgphoto_photopc.so -o libgphoto_photopc.so ${LIBOBJS}

libphotopc.a:	${LIBOBJS}
	${AR} r $@ $?
	${RANLIB} $@

install:
	${INSTALL} -d -m 755 ${LIBDEST}
	${INSTALL} -d -m 755 ${INCDEST}
	${INSTALL} -m 755 libeph_io.a ${LIBDEST}
	${INSTALL} -m 755 eph_io.h ${INCDEST}

clean:
	rm -f *.o libphotopc.a libgphoto_photopc.so

#eph_cmd.o: eph_io.h
#eph_cmd.o: eph_priv.h
#eph_err.o: eph_io.h
#eph_err.o: eph_priv.h
#eph_io.o: eph_io.h
#eph_io.o: eph_priv.h
#eph_io.o: usleep.h
#eph_io.o: comio.h
#eph_iob.o: eph_io.h
#eph_open.o: eph_io.h
#eph_open.o: eph_priv.h
#eph_open.o: usleep.h
#eph_open.o: comio.h
#eph_open.o: usleep.h
#eph_read.o: eph_io.h
#eph_read.o: comio.h
#eph_read.o: usleep.h
